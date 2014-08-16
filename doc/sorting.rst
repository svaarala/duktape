=======
Sorting
=======

This document summarizes the sorting algorithms used in the implementation.

Sorting algorithms are needed for:

* ``Array.prototype.sort()``

* Sorting array indexed keys for ``JSON.stringify()`` when an Array
  ``replacer`` is given

See:

* http://en.wikipedia.org/wiki/Sorting_algorithm#Comparison_of_algorithms

Array.prototype.sort()
======================

Some desirable qualities:

* Small code footprint

* Bounded C recursion

* In-place sorting (no large temporary allocations)

* O(n log n) average performance (weaker)

* O(n log n) worst case performance (stronger)

* Stability (don't reorder elements which compare equal)

C recursion is effectively bounded if recursion depth is O(log n) for a small
constant.  For instance, for 2^32 elements and a small constant, there would
be less than 100 recursive invocations which is good enough in most
environments.

``Array.prototype.sort()`` has some additional concerns:

* Value comparison is relatively expensive and involves coercion and/or
  comparison function calls.  Unless coercion results are cached, elements
  will usually be coerced multiple times.

* ``undefined`` and non-existent array elements need to be treated specially.
  This special behavior is encapsulated in the ``SortCompare()``
  specification algorithm.

* Exchanging elements is cheap for an object with an array part.

* Exchanging elements is very expensive for an object without an array
  part, requiring ``[[Get]]``, ``[[Put]]``, and ``[[Delete]]`` calls and
  may invoke setters/getters.

Current algorithm used is quick sort with randomized pivot selection.
Quick sort is O(n log n) average case but O(n^2) worst case, and is
not a stable sort.  Recursion depth is O(n) in the worst case.  There
is currently no fast path for array parts.

Random pivot is used to minimize the chance of the worst case O(n^2)
behavior without resorting to more complex (and large code footprint)
pivot selection algorithms.  Crafting malicious inputs to cause worst
case O(n^2) behavior is possible because the random number generator
is not cryptographic strength.

The current solution is a placeholder, and the sorting algorithm may
need to be changed entirely.  Getting a O(log n) recursion depth is
important because without it, sorting will fail in unlucky causes, as
it may be important to limit stack growth in some embedded environments.
Getting a O(n log n) worst case behavior would be nice, but not critical.

DUK_ENUM_SORT_ARRAY_INDICES
===========================

This sorting need arises in ``JSON.stringify()`` when the ``replacer``
argument is an Array.  In this case the replacer must be enumerated in
increasing array index order; E5.1 Section 15.12.3, first algorithm,
step 4.b.ii:

  For each value v of a property of replacer that has an array index
  property name. The properties are enumerated in the ascending array
  index order of their names.

This must hold even for sparse arrays, i.e. arrays without an array part.
Such arrays don't necessarily have their array index keys in the correct
ascending order, so they need to be sorted for enumeration.  The enumeration
API flag ``DUK_ENUM_SORT_ARRAY_INDICES`` provides the necessary behavior.
The expected data size for this sorting need is relatively small, and the
elements are expected to be almost always already in order.

Current algorithm used is insertion sort, which works well in-place and
handles already (almost) ordered cases efficiently.
