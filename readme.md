	My Own Kernel Drivers Study

	Usage:
		1. Copy mydrivers folder to your kernel source  drivers/
		2. add configs to top-level Kcongfig and Makefile
		3. make menuconfig

	Example:

diff --git a/drivers/Kconfig b/drivers/Kconfig
index ab6de06..234809a 100644
--- a/drivers/Kconfig
+++ b/drivers/Kconfig
@@ -1,5 +1,7 @@
 menu "Device Drivers"

+source "drivers/mydrivers/Kconfig"

 source "drivers/base/Kconfig"

 source "drivers/bus/Kconfig"
diff --git a/drivers/Makefile b/drivers/Makefile
index 8ca9a48..35298dc 100644
--- a/drivers/Makefile
+++ b/drivers/Makefile
@@ -4,6 +4,7 @@
 # 15 Sep 2000, Christoph Hellwig <hch@infradead.org>
 # Rewritten to use lists instead of if-statements.
 #

+obj-y  += mydrivers/

 obj-y                          += irqchip/
 obj-y                          += bus/
