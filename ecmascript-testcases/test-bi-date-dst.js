/* When converting an internal time value to local time, the DST offset is
 * determine based on the UTC time (i.e. DaylightSavingTA(t) is called, and
 * 't' is internal (UTC) time value).
 *
 * However, when converting back from local time to an internal time value,
 * DayLightSavingTA(t) is determined based on the local time (i.e.
 * DaylightSavingTA(t) is called, and 't' is a local time value).
 *
 * This has the impact, mentioned in E5.1 Section 15.9.1.9, that
 * UTC(LocalTime(t)) == t is not necessarily always true.
 *
 * XXX: Add a test for this for a specific timezone (e.g. Finnish).  There's
 * no direct access to plain UTC() or LocalTime() so the test needs to be
 * indirect.
 */

/*---
{
    "skip": true
}
---*/
