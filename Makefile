# Define paths to your target configurations
SDK_HW   := sdkconfig.hw
SDK_QEMU := sdkconfig

.PHONY: run qemu clean

# Targets Real Hardware: Forces the build system to use the hardware sdkconfig
#
run:
	idf.py build flash monitor

run-clean:
	# rm -f sdkconfig
	idf.py -DSDKCONFIG="$(SDK_HW)" reconfigure build flash monitor

# Targets QEMU Simulation: Forces the build system to use your qemu configuration
qemu:
	# rm -f sdkconfig
	idf.py -DSDKCONFIG="$(SDK_QEMU)" reconfigure qemu --qemu-extra-args="-nic user,model=open_eth,hostfwd=tcp:127.0.0.1:8080-:80" monitor

# Clean target to clear build caches when switching environments
clean:
	idf.py fullclean
	rm -f sdkconfig sdkconfig.old
