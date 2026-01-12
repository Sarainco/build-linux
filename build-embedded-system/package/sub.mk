# package/sub.mk

# 引入所有包的 mk
include $(PACKAGE)/busybox/sub.mk
# 以后可以自动化
# include $(wildcard $(PACKAGE)/*/*.mk)

# 2. 汇总目标
.PHONY: package
package: $(PACKAGE_TARGETS)
