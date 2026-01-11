host: host/kconfig

host/kconfig:
	[ -d $(HOST_OUTPUT) ] || mkdir -p $(HOST_OUTPUT)
	$(MAKE) -C $(HOST)/kconfig O=$(HOST_OUTPUT)/kconfig/

host/clean:
	rm -fr $(HOST_OUTPUT)

.PHONY: host/kconfig host/clean
