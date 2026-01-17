#include "camera.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <linux/videodev2.h>
#include <linux/v4l2-subdev.h>
#include <pthread.h>
#include <sys/select.h>
#include <time.h>
#include <stdint.h>
#include <linux/media.h>
#include <dirent.h>
#include <linux/videodev2.h>
#include <linux/v4l2-subdev.h>
#include <errno.h>


struct vbuf_t {
    unsigned char *buf;
    int size;
};

struct camera_t {
    int id; //第几个摄像头
    const char *dev;//设备节点
    int subdev_index;
    int fd;
    int subdev_fd;
    unsigned char opened; //是否open
    struct vbuf_t vbuf[VBUF_CNT];

    unsigned int total_gain;
    unsigned short width;
    unsigned short height;
};

struct camera_t my_cameras[] = {
    {
        .fd = 0,
        .dev = "/degv/video0",
        .subdev_index = 2,
        .fd = -1,
        .subdev_fd = -1,
        .opened = 0,
        .total_gain = 1000,
        .width = CAM_HEIGHT,
        .height = CAM_HEIGHT,
    },
};

void open_cam_subdev(struct camera_t *cam)
{
    if(cam->subdev_index < 0)
        return;

    char subdev_name[32] = {0};
    sprintf(subdev_name, "/dev/v4l-subdev%d", cam->subdev_index);

    printf("open_cam_subdev: %s\n", subdev_name);

    cam->subdev_fd = open(subdev_name, O_RDWR);
}

static void camera_open(struct camera_t *cam)
{
    int i;
    int ret;
    struct v4l2_capability cap;
    struct v4l2_format fmt;

    /*打开设备*/
    cam->fd = open(cam->dev, O_RDWR);
    if(cam->fd < 0) {
        printf("failed to open %s\n", cam->dev);
        return;
    }

    /*查询能力*/
    memset(&cap, 0, sizeof(struct v4l2_capability));
    ret = ioctl(cam->fd, VIDIOC_QUERYCAP, &cap);
    if(ret < 0) {
        printf("get caqpbility falied: %d\n", ret);
        close(cam->fd);
        cam->fd = -1;
    }

    /*获取当前格式*/
    memset(&fmt, 0, sizeof(struct v4l2_format));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    ret = ioctl(cam->fd, VIDIOC_G_FMT, &fmt);
    if(ret < 0) {
        printf("get fmt failed :%d\n", ret);
        close(cam->fd);
        cam->fd = -1;
    }

    //修改格式
    fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_SGRBG10;
    fmt.fmt.pix_mp.width = cam->width;
    fmt.fmt.pix_mp.height = cam->height;

    ret = ioctl(cam->fd, VIDIOC_S_FMT, &fmt);
    if(ret < 0) {
        printf("set fmt failed :%d\n", ret);
        close(cam->fd);
        cam->fd = -1;
    }

    //申请buffers
    struct v4l2_requestbuffers req = {
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
        .memory = V4L2_MEMORY_MMAP,
        .count = VBUF_CNT,//buffer count
    };

    ret = ioctl(cam->fd, VIDIOC_REQBUFS, &req);
    if(ret < 0) {
        printf("reqbufs failed :%d\n", ret);
        close(cam->fd);
        cam->fd = -1;
    }

    //建立内存映射
    struct v4l2_plane plane;
    struct v4l2_buffer buf;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.length = 1;
    buf.m.planes = &plane;

    for(buf.index = 0; buf.index < VBUF_CNT; buf.index++)
    {
        memset(&plane, 0, sizeof(struct v4l2_plane));
        ret = ioctl(cam->fd, VIDIOC_QUERYBUF, &buf);
        if(ret < 0) {
            printf("qbufs failed :%d\n", ret);
            close(cam->fd);
            cam->fd = -1;
        }

        cam->vbuf[buf.index].buf = mmap(NULL, plane.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                                        cam->fd, plane.m.mem_offset);
        if(cam->vbuf[buf.index].buf == MAP_FAILED) {
            printf("mmap failed\n");
            close(cam->fd);
            cam->fd = -1;
        }

        cam->vbuf[buf.index].size = plane.length;

    }

        // 将申请到的 buffer 投入到队列中;
    for(buf.index = 0; buf.index < VBUF_CNT; buf.index++)
    {
        if(0 != ioctl(cam->fd, VIDIOC_QBUF, &buf)) {
            printf("qbuf failed\n");
            munmap(cam->vbuf[buf.index].buf, cam->vbuf[buf.index].size);
            cam->vbuf[buf.index].buf = NULL;
            cam->vbuf[buf.index].size = 0;
            close(cam->fd);
            cam->fd = -1;
        }
    }

    cam->opened = 1;

    return;

}

static void camera_close(struct camera_t *cam)
{
    if(cam->fd < 0) {
        return;
    }

    int i;
    for(i = 0; i < VBUF_CNT; i++) {
        munmap(cam->vbuf[i].buf, cam->vbuf[i].size);
        cam->vbuf[i].buf = NULL;
        cam->vbuf[i].size = 0;
    }

    close(cam->fd);
    cam->fd = -1;
    cam->opened = 0;
}

