/* ============================================================
 * Control compile time options.
 * ============================================================
 *
 * Compile time options have moved to config.mk.
 */


/* ============================================================
 * Compatibility defines
 *
 * Generally for Windows native support.
 * ============================================================ */
#ifdef WIN32
#if _MSC_VER < 1900
#define snprintf sprintf_s
#endif
#define strcasecmp strcmpi
#define strtok_r strtok_s
#define strerror_r(e, b, l) strerror_s(b, l, e)
#endif
