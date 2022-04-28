--- /home/mike/code/cos/build/gcc-11.2.0/libstdc++-v3/crossconfig.m4	2021-07-28 00:55:09.228314429 -0600
+++ /home/mike/code/cos/build/gcc-alloy/libstdc++-v3/crossconfig.m4	2022-04-27 15:53:58.722847293 -0600
@@ -9,6 +9,13 @@
     # This is a freestanding configuration; there is nothing to do here.
     ;;
 
+  *-alloy*)
+    GLIBCXX_CHECK_COMPILER_FEATURES
+    GLIBCXX_CHECK_LINKER_FEATURES
+    GLIBCXX_CHECK_MATH_SUPPORT
+    GLIBCXX_CHECK_STDLIB_SUPPORT
+    ;;
+
   avr*-*-*)
     AC_DEFINE(HAVE_ACOSF)
     AC_DEFINE(HAVE_ASINF)
