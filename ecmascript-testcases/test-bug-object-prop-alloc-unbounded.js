/*
 *  There was an interesting bug up to Duktape 0.11.0, reported by Greg Burns.
 *
 *  With certain object property add/delete sequences the object property entry
 *  part can grow without bound, even though the object only has a limited
 *  number of actually used properties.
 *
 *  The root cause of this issue is the duk_hobject_props.c object entry part
 *  resize algorithm which computes the new entry part size based on the
 *  current allocated size (not *used size*).  Suppose that the entry part
 *  for keys looks as follows ('.' is unused):
 *
 *    [..........x]
 *
 *  if the object entry part is resized, a larger one is allocated with keys
 *  compacted:
 *
 *    [x...............]
 *
 *  with a suitable insert/delete sequence (like the simple one below), you
 *  can then again reach the case:
 *
 *    [...............x]
 *
 *  ... and resize to:
 *
 *    [x......................]
 *
 *  and so on.
 *
 *  The fix for this issue is to count the "used keys" explicitly before
 *  computing a new size for the object.  The actual reallocation primitive
 *  will run through the keys anyway to compact (and possibly rehash) them,
 *  so this is not a major issue performance-wise.
 */

/*===
without compaction
entry count < 100: true
entry count < 100: true
with compaction
entry count < 100: true
entry count < 100: true
===*/

function objectEntryPartResizeTest(doCompact) {
    var sparse = { 0: 0 };
    var i;
    var t, entryCount, entryNext;

    for (i = 1; i < 10000; i++) {
        sparse[i] = i;
        delete sparse[i - 1];

        if (doCompact) {
            /* If the object is forcibly compacted, the undesired entry
             * part behavior disappears even in Duktape 0.11.0 and prior.
             */
            Duktape.compact(sparse);
        }
    }

    /* Here the object entry part has been resized multiple times.  We
     * don't know the exact size, but since there is only one used key
     * we can more or less safely assume the entry part should be smaller
     * than 100 entries.
     *
     * NOTE: the "entry next" value is not the count of non-NULL key
     * entries in the entry part.  It's simply the next index to use
     * when adding a new key, and is not necessarily 1 here.
     */

    t = Duktape.info(sparse);
    //print(t);
    entryCount = t[5];  // version dependent, property entry allocated count
    entryNext = t[6];   // version dependent, property entry next index
    //print('entry next:', entryNext);
    //print('entry count:', entryCount);
    print('entry count < 100:', (entryCount < 100));

    /* With the bug present, a compaction "fixes" the object. */
    Duktape.compact(sparse);

    t = Duktape.info(sparse);
    //print(t);
    entryCount = t[5];  // version dependent, property entry allocated count
    entryNext = t[6];   // version dependent, property entry next index
    //print('entry next:', entryNext);
    //print('entry count:', entryCount);
    print('entry count < 100:', (entryCount < 100));
}

try {
    print('without compaction');
    objectEntryPartResizeTest(false);
    print('with compaction');
    objectEntryPartResizeTest(true);
} catch (e) {
    print(e);
}
