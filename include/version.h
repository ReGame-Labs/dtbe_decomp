#ifndef VERSION_H
#define VERSION_H

/*
 * The version of the game being built. The Makefile passes exactly one
 * -DVERSION_<VERSION> (make VERSION=us gives -DVERSION_US); here every
 * version's macro becomes 0 or 1, so that code tests them with #if:
 *
 *     #if VERSION_JP
 *     #if VERSION_US
 *
 * never with #ifdef or defined(), which can't tell a version that is off
 * from a misspelt one (-Wundef warns about the misspelling in #if). A new
 * version gets its line here and in the Makefile's VERSIONS.
 */
#if defined(VERSION_JP) + defined(VERSION_US) != 1
#error "build with exactly one of -DVERSION_JP and -DVERSION_US, as make VERSION=<version> does"
#endif

#ifndef VERSION_JP
#define VERSION_JP 0
#endif
#ifndef VERSION_US
#define VERSION_US 0
#endif

#endif /* VERSION_H */
