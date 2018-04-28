====================
Value stack resizing
====================

Growing and shrinking the value stack is important for both performance and
memory usage.  This document describes the mechanism in Duktape 2.2.

Value stack concepts
====================

The value stack is a single ``duk_tval`` array which is used by all calls in
the call stack of a certain Duktape thread.  The value stack has:

* An allocated size.  This is the actual memory allocation.

* A reserved size.  This is the value stack size that application or Duktape
  code has reserved.  The value stack cannot be shrunk below this size even in
  an emergency GC because calling code is expecting the reserved size to be
  guaranteed in all situations.

  - The current thread-level reserved size is the maximum reserved size for
    all activations in the call stack.  Call handling deals with updating the
    thread level reserve as calls are wound and unwound.

  - The reserve can only grow when calling functions because it would
    otherwise be unsafe to unwind protected calls as the unwind might run out
    memory.  That's a problem because either the unwind path would throw, or
    fail to respect the value stack guarantees for the reserve.  Both are
    sandboxing issues.  This strict policy technically only needs to be applied
    to protected calls, but it is applied to all calls at present.

  - The current reserve is also valid and tracked when the call stack is
    empty.  The reserve is then valid for the C code operating on the empty
    call stack.

* Current "top" of the value stack.  Value stack entries above top are not
  currently in use, and are always set to ECMAScript ``undefined`` as part
  of the value stack initialization policy.

  - Calling code is allowed to push values at the "top" index up to the
    current reserve.

  - It's not allowed to push values beyond the reserve, even if there is
    space between the reserve and the current allocation.  By default such
    pushes will throw; with ``DUK_USE_VALSTACK_UNSAFE`` (Lua-like behavior)
    such pushes will cause undefined behavior.

* Current "bottom" of the value stack.  This provides the basis for API call
  index resolution: index 0 is at the bottom.

  - When a call is in progress, "bottom" matches the functions' conceptual
    value stack frame.

  - When no call is in progress, "bottom" is zero.

Whenever side effects or value stack operations are possible, these values
must fulfill::

    0 <= bottom <= top <= reserve <= allocated
