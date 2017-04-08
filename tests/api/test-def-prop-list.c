/*
 *  Test duk_def_prop_list() and initializers.
 */

/*===
*** test_types (duk_safe_call)
My getter called, top=1, key=accessorValue
{
    undefinedValue: undefined,
    nullValue: null,
    booleanTrueValue: true,
    booleanFalseValue: false,
    numberValue: 123.456,
    stringValue: "NUL terminated string value",
    lstringValue: "string with\x00internal NULs",
    bufferWithoutInitValue: |000000000000000000000000000000000000|,
    bufferWithInitValue: |deadbeef12345678|,
    pointerValue: (0xdeadbeef),
    functionValue: {_func:true},
    lightfuncValue1: {_func:true},
    lightfuncValue2: {_func:true},
    accessorValue: "getter-returned-value",
    proplistValue: {
        name: "subObject",
        comment: "a new object is created for DUK_PROP_PROPLIST()"
    }
}
My function called, top-at-entry=3, magic=0, .length=3
My function called, top-at-entry=4, magic=0, .length=4
My function called, top-at-entry=4, magic=9, .length=2
My getter called, top=1, key=accessorValue
getter-returned-value
My setter called, top=2, key=accessorValue, value=foobar
final top: 0
==> rc=0, result='undefined'
===*/

/* Test basic types. */

static duk_ret_t my_function(duk_context *ctx) {
	duk_push_current_function(ctx);
	duk_get_prop_string(ctx, -1, "length");
	printf("My function called, top-at-entry=%ld, magic=%ld, .length=%s\n",
	       (long) duk_get_top(ctx) - 2, (long) duk_get_current_magic(ctx), duk_safe_to_string(ctx, -1));
	return 0;
}

static duk_ret_t my_getter(duk_context *ctx) {
	printf("My getter called, top=%ld, key=%s\n",
	       (long) duk_get_top(ctx), duk_safe_to_string(ctx, 0));
	duk_push_string(ctx, "getter-returned-value");
	return 1;
}

static duk_ret_t my_setter(duk_context *ctx) {
	printf("My setter called, top=%ld, key=%s, value=%s\n",
	       (long) duk_get_top(ctx), duk_safe_to_string(ctx, 1), duk_safe_to_string(ctx, 0));
	return 0;
}

static duk_ret_t test_types(duk_context *ctx) {
	unsigned char buf_data[10] = {
		0xde, 0xad, 0xbe, 0xef, 0x12, 0x34, 0x56, 0x78, 0xaa, 0xbb  /* 0xaa and 0xbb ignored because of length */
	};
	duk_prop_list_entry sub_props[] = {
		DUK_PROP_STRING("name", "subObject"),
		DUK_PROP_STRING("comment", "a new object is created for DUK_PROP_PROPLIST()"),
		DUK_PROP_END()
	};
	duk_prop_list_entry props[] = {
		DUK_PROP_UNDEFINED("undefinedValue"),
		DUK_PROP_NULL("nullValue"),
		DUK_PROP_BOOLEAN("booleanTrueValue", 123),  /* any nonzero */
		DUK_PROP_BOOLEAN("booleanFalseValue", 0),
		DUK_PROP_NUMBER("numberValue", 123.456),
		DUK_PROP_STRING("stringValue", "NUL terminated string value"),
		DUK_PROP_LSTRING("lstringValue", "string with\x00internal NULs[IGNORED]", 25),
		/* FIXME: object */
		DUK_PROP_BUFFER("bufferWithoutInitValue", NULL, 18),  /* fixed buffer without initialization data */
		DUK_PROP_BUFFER("bufferWithInitValue", buf_data, 8),  /* fixed buffer with initialization data */
		DUK_PROP_POINTER("pointerValue", (void *) 0xdeadbeef),
		DUK_PROP_FUNCTION("functionValue", my_function, 3 /*nargs*/),
		DUK_PROP_LIGHTFUNC("lightfuncValue1", my_function, 4 /*nargs*/),
		DUK_PROP_LIGHTFUNC_LENGTH_MAGIC("lightfuncValue2", my_function, 4 /*nargs*/, 2 /*length*/, 9 /*magic*/),
		DUK_PROP_ACCESSOR("accessorValue", my_getter, my_setter),
		DUK_PROP_PROPLIST("proplistValue", sub_props),
		DUK_PROP_END()
	};

	duk_push_object(ctx);
	duk_def_prop_list(ctx, -1, props);

	duk_eval_string(ctx, "(function (v) { print(Duktape.enc('jx', v, null, 4)); })");
	duk_dup(ctx, 0);
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_eval_string(ctx,
		"(function (v) {\n"
		"    v.functionValue('foo', 'bar', 'quux', 'baz', 'quuux');\n"
		"    v.lightfuncValue1('foo', 'bar', 'quux', 'baz', 'quuux');\n"
		"    v.lightfuncValue2('foo', 'bar', 'quux', 'baz', 'quuux');\n"
		"    print(v.accessorValue);\n"
		"    v.accessorValue = 'foobar';\n"
		"})");
	duk_dup(ctx, 0);
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_deep_structure (duk_safe_call)
{
 index: 0,
 child: {
  index: 1,
  child: {
   index: 2,
   child: {
    index: 3,
    child: {
     index: 4,
     child: {
      index: 5,
      child: {
       index: 6,
       child: {
        index: 7,
        child: {
         index: 8,
         child: {
          index: 9,
          child: {
           index: 10,
           child: {
            index: 11,
            child: {
             index: 12,
             child: {
              index: 13,
              child: {
               index: 14,
               child: {
                index: 15,
                child: {
                 index: 16,
                 child: {
                  index: 17,
                  child: {
                   index: 18,
                   child: {
                    index: 19,
                    child: {
                     index: 20,
                     child: {
                      index: 21,
                      child: {
                       index: 22,
                       child: {
                        index: 23,
                        child: {
                         index: 24,
                         child: {
                          index: 25,
                          child: {
                           index: 26,
                           child: {
                            index: 27,
                            child: {
                             index: 28,
                             child: {
                              index: 29,
                              child: {
                               index: 30,
                               child: {
                                index: 31,
                                child: {
                                 index: 32,
                                 child: {
                                  index: 33,
                                  child: {
                                   index: 34,
                                   child: {
                                    index: 35,
                                    child: {
                                     index: 36,
                                     child: {
                                      index: 37,
                                      child: {
                                       index: 38,
                                       child: {
                                        index: 39,
                                        child: {
                                         index: 40,
                                         child: {
                                          index: 41,
                                          child: {
                                           index: 42,
                                           child: {
                                            index: 43,
                                            child: {
                                             index: 44,
                                             child: {
                                              index: 45,
                                              child: {
                                               index: 46,
                                               child: {
                                                index: 47,
                                                child: {
                                                 index: 48,
                                                 child: {
                                                  index: 49,
                                                  child: {
                                                   index: 50,
                                                   child: {
                                                    index: 51,
                                                    child: {
                                                     index: 52,
                                                     child: {
                                                      index: 53,
                                                      child: {
                                                       index: 54,
                                                       child: {
                                                        index: 55,
                                                        child: {
                                                         index: 56,
                                                         child: {
                                                          index: 57,
                                                          child: {
                                                           index: 58,
                                                           child: {
                                                            index: 59,
                                                            child: {
                                                             index: 60,
                                                             child: {
                                                              index: 61,
                                                              child: {
                                                               index: 62,
                                                               child: {
                                                                index: 63,
                                                                child: {
                                                                 index: 64,
                                                                 child: {
                                                                  index: 65,
                                                                  child: {
                                                                   index: 66,
                                                                   child: {
                                                                    index: 67,
                                                                    child: {
                                                                     index: 68,
                                                                     child: {
                                                                      index: 69,
                                                                      child: {
                                                                       index: 70,
                                                                       child: {
                                                                        index: 71,
                                                                        child: {
                                                                         index: 72,
                                                                         child: {
                                                                          index: 73,
                                                                          child: {
                                                                           index: 74,
                                                                           child: {
                                                                            index: 75,
                                                                            child: {
                                                                             index: 76,
                                                                             child: {
                                                                              index: 77,
                                                                              child: {
                                                                               index: 78,
                                                                               child: {
                                                                                index: 79,
                                                                                child: {
                                                                                 index: 80,
                                                                                 child: {
                                                                                  index: 81,
                                                                                  child: {
                                                                                   index: 82,
                                                                                   child: {
                                                                                    index: 83,
                                                                                    child: {
                                                                                     index: 84,
                                                                                     child: {
                                                                                      index: 85,
                                                                                      child: {
                                                                                       index: 86,
                                                                                       child: {
                                                                                        index: 87,
                                                                                        child: {
                                                                                         index: 88,
                                                                                         child: {
                                                                                          index: 89,
                                                                                          child: {
                                                                                           index: 90,
                                                                                           child: {
                                                                                            index: 91,
                                                                                            child: {
                                                                                             index: 92,
                                                                                             child: {
                                                                                              index: 93,
                                                                                              child: {
                                                                                               index: 94,
                                                                                               child: {
                                                                                                index: 95,
                                                                                                child: {
                                                                                                 index: 96,
                                                                                                 child: {
                                                                                                  index: 97,
                                                                                                  child: {
                                                                                                   index: 98,
                                                                                                   child: {
                                                                                                    index: 99,
                                                                                                    child: {
                                                                                                     index: 100,
                                                                                                     child: {
                                                                                                      index: 101,
                                                                                                      child: {
                                                                                                       index: 102,
                                                                                                       child: {
                                                                                                        index: 103,
                                                                                                        child: {
                                                                                                         index: 104,
                                                                                                         child: {
                                                                                                          index: 105,
                                                                                                          child: {
                                                                                                           index: 106,
                                                                                                           child: {
                                                                                                            index: 107,
                                                                                                            child: {
                                                                                                             index: 108,
                                                                                                             child: {
                                                                                                              index: 109,
                                                                                                              child: {
                                                                                                               index: 110,
                                                                                                               child: {
                                                                                                                index: 111,
                                                                                                                child: {
                                                                                                                 index: 112,
                                                                                                                 child: {
                                                                                                                  index: 113,
                                                                                                                  child: {
                                                                                                                   index: 114,
                                                                                                                   child: {
                                                                                                                    index: 115,
                                                                                                                    child: {
                                                                                                                     index: 116,
                                                                                                                     child: {
                                                                                                                      index: 117,
                                                                                                                      child: {
                                                                                                                       index: 118,
                                                                                                                       child: {
                                                                                                                        index: 119,
                                                                                                                        child: {
                                                                                                                         index: 120,
                                                                                                                         child: {
                                                                                                                          index: 121,
                                                                                                                          child: {
                                                                                                                           index: 122,
                                                                                                                           child: {
                                                                                                                            index: 123,
                                                                                                                            child: {
                                                                                                                             index: 124,
                                                                                                                             child: {
                                                                                                                              index: 125,
                                                                                                                              child: {
                                                                                                                               index: 126,
                                                                                                                               child: {
                                                                                                                                index: 127,
                                                                                                                                child: {
                                                                                                                                 index: 128,
                                                                                                                                 child: {
                                                                                                                                  index: 129,
                                                                                                                                  child: {
                                                                                                                                   name: "deepest"
                                                                                                                                  }
                                                                                                                                 }
                                                                                                                                }
                                                                                                                               }
                                                                                                                              }
                                                                                                                             }
                                                                                                                            }
                                                                                                                           }
                                                                                                                          }
                                                                                                                         }
                                                                                                                        }
                                                                                                                       }
                                                                                                                      }
                                                                                                                     }
                                                                                                                    }
                                                                                                                   }
                                                                                                                  }
                                                                                                                 }
                                                                                                                }
                                                                                                               }
                                                                                                              }
                                                                                                             }
                                                                                                            }
                                                                                                           }
                                                                                                          }
                                                                                                         }
                                                                                                        }
                                                                                                       }
                                                                                                      }
                                                                                                     }
                                                                                                    }
                                                                                                   }
                                                                                                  }
                                                                                                 }
                                                                                                }
                                                                                               }
                                                                                              }
                                                                                             }
                                                                                            }
                                                                                           }
                                                                                          }
                                                                                         }
                                                                                        }
                                                                                       }
                                                                                      }
                                                                                     }
                                                                                    }
                                                                                   }
                                                                                  }
                                                                                 }
                                                                                }
                                                                               }
                                                                              }
                                                                             }
                                                                            }
                                                                           }
                                                                          }
                                                                         }
                                                                        }
                                                                       }
                                                                      }
                                                                     }
                                                                    }
                                                                   }
                                                                  }
                                                                 }
                                                                }
                                                               }
                                                              }
                                                             }
                                                            }
                                                           }
                                                          }
                                                         }
                                                        }
                                                       }
                                                      }
                                                     }
                                                    }
                                                   }
                                                  }
                                                 }
                                                }
                                               }
                                              }
                                             }
                                            }
                                           }
                                          }
                                         }
                                        }
                                       }
                                      }
                                     }
                                    }
                                   }
                                  }
                                 }
                                }
                               }
                              }
                             }
                            }
                           }
                          }
                         }
                        }
                       }
                      }
                     }
                    }
                   }
                  }
                 }
                }
               }
              }
             }
            }
           }
          }
         }
        }
       }
      }
     }
    }
   }
  }
 }
}
final top: 0
==> rc=0, result='undefined'
===*/

/* Test a deeply nested structure.  This ensures that duk_def_prop_list() does
 * a stack extend check when it recurses (otherwise we'd run out of stack space
 * at around depth 64-128).
 */

#define MKPROPS(idx,idx_child) \
	static duk_prop_list_entry p##idx[] = { DUK_PROP_NUMBER("index", (idx)), DUK_PROP_PROPLIST("child", p##idx_child), DUK_PROP_END() }

static duk_prop_list_entry p130[] = { DUK_PROP_STRING("name", "deepest"), DUK_PROP_END() };
MKPROPS(129, 130); MKPROPS(128, 129); MKPROPS(127, 128); MKPROPS(126, 127); MKPROPS(125, 126);
MKPROPS(124, 125); MKPROPS(123, 124); MKPROPS(122, 123); MKPROPS(121, 122); MKPROPS(120, 121);
MKPROPS(119, 120); MKPROPS(118, 119); MKPROPS(117, 118); MKPROPS(116, 117); MKPROPS(115, 116);
MKPROPS(114, 115); MKPROPS(113, 114); MKPROPS(112, 113); MKPROPS(111, 112); MKPROPS(110, 111);
MKPROPS(109, 110); MKPROPS(108, 109); MKPROPS(107, 108); MKPROPS(106, 107); MKPROPS(105, 106);
MKPROPS(104, 105); MKPROPS(103, 104); MKPROPS(102, 103); MKPROPS(101, 102); MKPROPS(100, 101);
MKPROPS(99, 100); MKPROPS(98, 99); MKPROPS(97, 98); MKPROPS(96, 97); MKPROPS(95, 96);
MKPROPS(94, 95); MKPROPS(93, 94); MKPROPS(92, 93); MKPROPS(91, 92); MKPROPS(90, 91);
MKPROPS(89, 90); MKPROPS(88, 89); MKPROPS(87, 88); MKPROPS(86, 87); MKPROPS(85, 86);
MKPROPS(84, 85); MKPROPS(83, 84); MKPROPS(82, 83); MKPROPS(81, 82); MKPROPS(80, 81);
MKPROPS(79, 80); MKPROPS(78, 79); MKPROPS(77, 78); MKPROPS(76, 77); MKPROPS(75, 76);
MKPROPS(74, 75); MKPROPS(73, 74); MKPROPS(72, 73); MKPROPS(71, 72); MKPROPS(70, 71);
MKPROPS(69, 70); MKPROPS(68, 69); MKPROPS(67, 68); MKPROPS(66, 67); MKPROPS(65, 66);
MKPROPS(64, 65); MKPROPS(63, 64); MKPROPS(62, 63); MKPROPS(61, 62); MKPROPS(60, 61);
MKPROPS(59, 60); MKPROPS(58, 59); MKPROPS(57, 58); MKPROPS(56, 57); MKPROPS(55, 56);
MKPROPS(54, 55); MKPROPS(53, 54); MKPROPS(52, 53); MKPROPS(51, 52); MKPROPS(50, 51);
MKPROPS(49, 50); MKPROPS(48, 49); MKPROPS(47, 48); MKPROPS(46, 47); MKPROPS(45, 46);
MKPROPS(44, 45); MKPROPS(43, 44); MKPROPS(42, 43); MKPROPS(41, 42); MKPROPS(40, 41);
MKPROPS(39, 40); MKPROPS(38, 39); MKPROPS(37, 38); MKPROPS(36, 37); MKPROPS(35, 36);
MKPROPS(34, 35); MKPROPS(33, 34); MKPROPS(32, 33); MKPROPS(31, 32); MKPROPS(30, 31);
MKPROPS(29, 30); MKPROPS(28, 29); MKPROPS(27, 28); MKPROPS(26, 27); MKPROPS(25, 26);
MKPROPS(24, 25); MKPROPS(23, 24); MKPROPS(22, 23); MKPROPS(21, 22); MKPROPS(20, 21);
MKPROPS(19, 20); MKPROPS(18, 19); MKPROPS(17, 18); MKPROPS(16, 17); MKPROPS(15, 16);
MKPROPS(14, 15); MKPROPS(13, 14); MKPROPS(12, 13); MKPROPS(11, 12); MKPROPS(10, 11);
MKPROPS(9, 10); MKPROPS(8, 9); MKPROPS(7, 8); MKPROPS(6, 7); MKPROPS(5, 6);
MKPROPS(4, 5); MKPROPS(3, 4); MKPROPS(2, 3); MKPROPS(1, 2); MKPROPS(0, 1);

static duk_ret_t test_deep_structure(duk_context *ctx) {
	duk_push_object(ctx);
	duk_def_prop_list(ctx, -1, p0);

	duk_eval_string(ctx, "(function (v) { print(Duktape.enc('jx', v, null, 1)); })");
	duk_dup(ctx, 0);
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*
 *  Main
 */

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_types);
	TEST_SAFE_CALL(test_deep_structure);
}

/* FIXME: index check */
