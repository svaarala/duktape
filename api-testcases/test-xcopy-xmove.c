/*
 *  duk_xcopy_test() and duk_xmove_test()
 *
 *  Test finalizer/refcount handling carefully, as it's easy to break in
 *  the implementation.
 */

/*===
*** test_xcopy_top_basic (duk_safe_call)
ctx1 (top=10): foo0 foo1 foo2 foo3 foo4 foo5 foo6 foo7 foo8 foo9
ctx2 (top=8): bar0 bar1 bar2 bar3 bar4 bar5 bar6 bar7
ctx1 (top=10): foo0 foo1 foo2 foo3 foo4 foo5 foo6 foo7 foo8 foo9
ctx2 (top=8): bar0 bar1 bar2 bar3 bar4 bar5 bar6 bar7
ctx1 (top=13): foo0 foo1 foo2 foo3 foo4 foo5 foo6 foo7 foo8 foo9 bar5 bar6 bar7
ctx2 (top=8): bar0 bar1 bar2 bar3 bar4 bar5 bar6 bar7
ctx1 (top=21): foo0 foo1 foo2 foo3 foo4 foo5 foo6 foo7 foo8 foo9 bar5 bar6 bar7 bar0 bar1 bar2 bar3 bar4 bar5 bar6 bar7
ctx2 (top=8): bar0 bar1 bar2 bar3 bar4 bar5 bar6 bar7
final top: 2
==> rc=0, result='undefined'
*** test_xcopy_top_large (duk_safe_call)
ctx1 top: 180000
ctx2 top: 80000
final top: 2
==> rc=0, result='undefined'
*** test_xcopy_top_refcount (duk_safe_call)
ctx1 (top=10): [foo-0] [foo-1] [foo-2] [foo-3] [foo-4] [foo-5] [foo-6] [foo-7] [foo-8] [foo-9]
ctx2 (top=8): [bar-0] [bar-1] [bar-2] [bar-3] [bar-4] [bar-5] [bar-6] [bar-7]
ctx1 (top=10): [foo-0] [foo-1] [foo-2] [foo-3] [foo-4] [foo-5] [foo-6] [foo-7] [foo-8] [foo-9]
ctx2 (top=8): [bar-0] [bar-1] [bar-2] [bar-3] [bar-4] [bar-5] [bar-6] [bar-7]
ctx1 (top=13): [foo-0] [foo-1] [foo-2] [foo-3] [foo-4] [foo-5] [foo-6] [foo-7] [foo-8] [foo-9] [bar-5] [bar-6] [bar-7]
ctx2 (top=8): [bar-0] [bar-1] [bar-2] [bar-3] [bar-4] [bar-5] [bar-6] [bar-7]
ctx1 (top=21): [foo-0] [foo-1] [foo-2] [foo-3] [foo-4] [foo-5] [foo-6] [foo-7] [foo-8] [foo-9] [bar-5] [bar-6] [bar-7] [bar-0] [bar-1] [bar-2] [bar-3] [bar-4] [bar-5] [bar-6] [bar-7]
ctx2 (top=8): [bar-0] [bar-1] [bar-2] [bar-3] [bar-4] [bar-5] [bar-6] [bar-7]
set top for ctx2 to 7
set top for ctx2 to 6
set top for ctx2 to 5
set top for ctx2 to 4
set top for ctx2 to 3
set top for ctx2 to 2
set top for ctx2 to 1
set top for ctx2 to 0
set top for ctx1 to 20
set top for ctx1 to 19
set top for ctx1 to 18
set top for ctx1 to 17
fin bar-4
set top for ctx1 to 16
fin bar-3
set top for ctx1 to 15
fin bar-2
set top for ctx1 to 14
fin bar-1
set top for ctx1 to 13
fin bar-0
set top for ctx1 to 12
fin bar-7
set top for ctx1 to 11
fin bar-6
set top for ctx1 to 10
fin bar-5
set top for ctx1 to 9
fin foo-9
set top for ctx1 to 8
fin foo-8
set top for ctx1 to 7
fin foo-7
set top for ctx1 to 6
fin foo-6
set top for ctx1 to 5
fin foo-5
set top for ctx1 to 4
fin foo-4
set top for ctx1 to 3
fin foo-3
set top for ctx1 to 2
fin foo-2
set top for ctx1 to 1
fin foo-1
set top for ctx1 to 0
fin foo-0
final top: 2
==> rc=0, result='undefined'
*** test_xcopy_top_samectx (duk_safe_call)
ctx1 (top=10): foo0 foo1 foo2 foo3 foo4 foo5 foo6 foo7 foo8 foo9
ctx2 (top=8): bar0 bar1 bar2 bar3 bar4 bar5 bar6 bar7
==> rc=1, result='Error: invalid context'
*** test_xcopy_top_negcount (duk_safe_call)
ctx1 (top=10): foo0 foo1 foo2 foo3 foo4 foo5 foo6 foo7 foo8 foo9
ctx2 (top=8): bar0 bar1 bar2 bar3 bar4 bar5 bar6 bar7
==> rc=1, result='Error: invalid count'
*** test_xcopy_top_verylargecount (duk_safe_call)
ctx1 (top=10): foo0 foo1 foo2 foo3 foo4 foo5 foo6 foo7 foo8 foo9
ctx2 (top=8): bar0 bar1 bar2 bar3 bar4 bar5 bar6 bar7
==> rc=1, result='Error: invalid count'
*** test_xcopy_top_notenoughsrc (duk_safe_call)
ctx1 (top=10): foo0 foo1 foo2 foo3 foo4 foo5 foo6 foo7 foo8 foo9
ctx2 (top=8): bar0 bar1 bar2 bar3 bar4 bar5 bar6 bar7
==> rc=1, result='Error: invalid count'
*** test_xcopy_top_notenoughdst (duk_safe_call)
ctx1 (top=10): foo0 foo1 foo2 foo3 foo4 foo5 foo6 foo7 foo8 foo9
ctx2 (top=1008): bar0 bar1 bar2 bar3 bar4 bar5 bar6 bar7 str-0 str-1 str-2 str-3 str-4 str-5 str-6 str-7 str-8 str-9 str-10 str-11 str-12 str-13 str-14 str-15 str-16 str-17 str-18 str-19 str-20 str-21 str-22 str-23 str-24 str-25 str-26 str-27 str-28 str-29 str-30 str-31 str-32 str-33 str-34 str-35 str-36 str-37 str-38 str-39 str-40 str-41 str-42 str-43 str-44 str-45 str-46 str-47 str-48 str-49 str-50 str-51 str-52 str-53 str-54 str-55 str-56 str-57 str-58 str-59 str-60 str-61 str-62 str-63 str-64 str-65 str-66 str-67 str-68 str-69 str-70 str-71 str-72 str-73 str-74 str-75 str-76 str-77 str-78 str-79 str-80 str-81 str-82 str-83 str-84 str-85 str-86 str-87 str-88 str-89 str-90 str-91 str-92 str-93 str-94 str-95 str-96 str-97 str-98 str-99 str-100 str-101 str-102 str-103 str-104 str-105 str-106 str-107 str-108 str-109 str-110 str-111 str-112 str-113 str-114 str-115 str-116 str-117 str-118 str-119 str-120 str-121 str-122 str-123 str-124 str-125 str-126 str-127 str-128 str-129 str-130 str-131 str-132 str-133 str-134 str-135 str-136 str-137 str-138 str-139 str-140 str-141 str-142 str-143 str-144 str-145 str-146 str-147 str-148 str-149 str-150 str-151 str-152 str-153 str-154 str-155 str-156 str-157 str-158 str-159 str-160 str-161 str-162 str-163 str-164 str-165 str-166 str-167 str-168 str-169 str-170 str-171 str-172 str-173 str-174 str-175 str-176 str-177 str-178 str-179 str-180 str-181 str-182 str-183 str-184 str-185 str-186 str-187 str-188 str-189 str-190 str-191 str-192 str-193 str-194 str-195 str-196 str-197 str-198 str-199 str-200 str-201 str-202 str-203 str-204 str-205 str-206 str-207 str-208 str-209 str-210 str-211 str-212 str-213 str-214 str-215 str-216 str-217 str-218 str-219 str-220 str-221 str-222 str-223 str-224 str-225 str-226 str-227 str-228 str-229 str-230 str-231 str-232 str-233 str-234 str-235 str-236 str-237 str-238 str-239 str-240 str-241 str-242 str-243 str-244 str-245 str-246 str-247 str-248 str-249 str-250 str-251 str-252 str-253 str-254 str-255 str-256 str-257 str-258 str-259 str-260 str-261 str-262 str-263 str-264 str-265 str-266 str-267 str-268 str-269 str-270 str-271 str-272 str-273 str-274 str-275 str-276 str-277 str-278 str-279 str-280 str-281 str-282 str-283 str-284 str-285 str-286 str-287 str-288 str-289 str-290 str-291 str-292 str-293 str-294 str-295 str-296 str-297 str-298 str-299 str-300 str-301 str-302 str-303 str-304 str-305 str-306 str-307 str-308 str-309 str-310 str-311 str-312 str-313 str-314 str-315 str-316 str-317 str-318 str-319 str-320 str-321 str-322 str-323 str-324 str-325 str-326 str-327 str-328 str-329 str-330 str-331 str-332 str-333 str-334 str-335 str-336 str-337 str-338 str-339 str-340 str-341 str-342 str-343 str-344 str-345 str-346 str-347 str-348 str-349 str-350 str-351 str-352 str-353 str-354 str-355 str-356 str-357 str-358 str-359 str-360 str-361 str-362 str-363 str-364 str-365 str-366 str-367 str-368 str-369 str-370 str-371 str-372 str-373 str-374 str-375 str-376 str-377 str-378 str-379 str-380 str-381 str-382 str-383 str-384 str-385 str-386 str-387 str-388 str-389 str-390 str-391 str-392 str-393 str-394 str-395 str-396 str-397 str-398 str-399 str-400 str-401 str-402 str-403 str-404 str-405 str-406 str-407 str-408 str-409 str-410 str-411 str-412 str-413 str-414 str-415 str-416 str-417 str-418 str-419 str-420 str-421 str-422 str-423 str-424 str-425 str-426 str-427 str-428 str-429 str-430 str-431 str-432 str-433 str-434 str-435 str-436 str-437 str-438 str-439 str-440 str-441 str-442 str-443 str-444 str-445 str-446 str-447 str-448 str-449 str-450 str-451 str-452 str-453 str-454 str-455 str-456 str-457 str-458 str-459 str-460 str-461 str-462 str-463 str-464 str-465 str-466 str-467 str-468 str-469 str-470 str-471 str-472 str-473 str-474 str-475 str-476 str-477 str-478 str-479 str-480 str-481 str-482 str-483 str-484 str-485 str-486 str-487 str-488 str-489 str-490 str-491 str-492 str-493 str-494 str-495 str-496 str-497 str-498 str-499 str-500 str-501 str-502 str-503 str-504 str-505 str-506 str-507 str-508 str-509 str-510 str-511 str-512 str-513 str-514 str-515 str-516 str-517 str-518 str-519 str-520 str-521 str-522 str-523 str-524 str-525 str-526 str-527 str-528 str-529 str-530 str-531 str-532 str-533 str-534 str-535 str-536 str-537 str-538 str-539 str-540 str-541 str-542 str-543 str-544 str-545 str-546 str-547 str-548 str-549 str-550 str-551 str-552 str-553 str-554 str-555 str-556 str-557 str-558 str-559 str-560 str-561 str-562 str-563 str-564 str-565 str-566 str-567 str-568 str-569 str-570 str-571 str-572 str-573 str-574 str-575 str-576 str-577 str-578 str-579 str-580 str-581 str-582 str-583 str-584 str-585 str-586 str-587 str-588 str-589 str-590 str-591 str-592 str-593 str-594 str-595 str-596 str-597 str-598 str-599 str-600 str-601 str-602 str-603 str-604 str-605 str-606 str-607 str-608 str-609 str-610 str-611 str-612 str-613 str-614 str-615 str-616 str-617 str-618 str-619 str-620 str-621 str-622 str-623 str-624 str-625 str-626 str-627 str-628 str-629 str-630 str-631 str-632 str-633 str-634 str-635 str-636 str-637 str-638 str-639 str-640 str-641 str-642 str-643 str-644 str-645 str-646 str-647 str-648 str-649 str-650 str-651 str-652 str-653 str-654 str-655 str-656 str-657 str-658 str-659 str-660 str-661 str-662 str-663 str-664 str-665 str-666 str-667 str-668 str-669 str-670 str-671 str-672 str-673 str-674 str-675 str-676 str-677 str-678 str-679 str-680 str-681 str-682 str-683 str-684 str-685 str-686 str-687 str-688 str-689 str-690 str-691 str-692 str-693 str-694 str-695 str-696 str-697 str-698 str-699 str-700 str-701 str-702 str-703 str-704 str-705 str-706 str-707 str-708 str-709 str-710 str-711 str-712 str-713 str-714 str-715 str-716 str-717 str-718 str-719 str-720 str-721 str-722 str-723 str-724 str-725 str-726 str-727 str-728 str-729 str-730 str-731 str-732 str-733 str-734 str-735 str-736 str-737 str-738 str-739 str-740 str-741 str-742 str-743 str-744 str-745 str-746 str-747 str-748 str-749 str-750 str-751 str-752 str-753 str-754 str-755 str-756 str-757 str-758 str-759 str-760 str-761 str-762 str-763 str-764 str-765 str-766 str-767 str-768 str-769 str-770 str-771 str-772 str-773 str-774 str-775 str-776 str-777 str-778 str-779 str-780 str-781 str-782 str-783 str-784 str-785 str-786 str-787 str-788 str-789 str-790 str-791 str-792 str-793 str-794 str-795 str-796 str-797 str-798 str-799 str-800 str-801 str-802 str-803 str-804 str-805 str-806 str-807 str-808 str-809 str-810 str-811 str-812 str-813 str-814 str-815 str-816 str-817 str-818 str-819 str-820 str-821 str-822 str-823 str-824 str-825 str-826 str-827 str-828 str-829 str-830 str-831 str-832 str-833 str-834 str-835 str-836 str-837 str-838 str-839 str-840 str-841 str-842 str-843 str-844 str-845 str-846 str-847 str-848 str-849 str-850 str-851 str-852 str-853 str-854 str-855 str-856 str-857 str-858 str-859 str-860 str-861 str-862 str-863 str-864 str-865 str-866 str-867 str-868 str-869 str-870 str-871 str-872 str-873 str-874 str-875 str-876 str-877 str-878 str-879 str-880 str-881 str-882 str-883 str-884 str-885 str-886 str-887 str-888 str-889 str-890 str-891 str-892 str-893 str-894 str-895 str-896 str-897 str-898 str-899 str-900 str-901 str-902 str-903 str-904 str-905 str-906 str-907 str-908 str-909 str-910 str-911 str-912 str-913 str-914 str-915 str-916 str-917 str-918 str-919 str-920 str-921 str-922 str-923 str-924 str-925 str-926 str-927 str-928 str-929 str-930 str-931 str-932 str-933 str-934 str-935 str-936 str-937 str-938 str-939 str-940 str-941 str-942 str-943 str-944 str-945 str-946 str-947 str-948 str-949 str-950 str-951 str-952 str-953 str-954 str-955 str-956 str-957 str-958 str-959 str-960 str-961 str-962 str-963 str-964 str-965 str-966 str-967 str-968 str-969 str-970 str-971 str-972 str-973 str-974 str-975 str-976 str-977 str-978 str-979 str-980 str-981 str-982 str-983 str-984 str-985 str-986 str-987 str-988 str-989 str-990 str-991 str-992 str-993 str-994 str-995 str-996 str-997 str-998 str-999
==> rc=1, result='Error: attempt to push beyond currently allocated stack'
*** test_xmove_top_basic (duk_safe_call)
ctx1 (top=10): foo0 foo1 foo2 foo3 foo4 foo5 foo6 foo7 foo8 foo9
ctx2 (top=8): bar0 bar1 bar2 bar3 bar4 bar5 bar6 bar7
ctx1 (top=13): foo0 foo1 foo2 foo3 foo4 foo5 foo6 foo7 foo8 foo9 bar5 bar6 bar7
ctx2 (top=5): bar0 bar1 bar2 bar3 bar4
ctx1 (top=18): foo0 foo1 foo2 foo3 foo4 foo5 foo6 foo7 foo8 foo9 bar5 bar6 bar7 bar0 bar1 bar2 bar3 bar4
ctx2 (top=0):
final top: 2
==> rc=0, result='undefined'
*** test_xmove_top_large (duk_safe_call)
ctx1 top: 180000
ctx2 top: 0
final top: 2
==> rc=0, result='undefined'
*** test_xmove_top_refcount (duk_safe_call)
ctx1 (top=10): [foo-0] [foo-1] [foo-2] [foo-3] [foo-4] [foo-5] [foo-6] [foo-7] [foo-8] [foo-9]
ctx2 (top=8): [bar-0] [bar-1] [bar-2] [bar-3] [bar-4] [bar-5] [bar-6] [bar-7]
ctx1 (top=13): [foo-0] [foo-1] [foo-2] [foo-3] [foo-4] [foo-5] [foo-6] [foo-7] [foo-8] [foo-9] [bar-5] [bar-6] [bar-7]
ctx2 (top=5): [bar-0] [bar-1] [bar-2] [bar-3] [bar-4]
ctx1 (top=18): [foo-0] [foo-1] [foo-2] [foo-3] [foo-4] [foo-5] [foo-6] [foo-7] [foo-8] [foo-9] [bar-5] [bar-6] [bar-7] [bar-0] [bar-1] [bar-2] [bar-3] [bar-4]
ctx2 (top=0):
set top for ctx1 to 17
fin bar-4
set top for ctx1 to 16
fin bar-3
set top for ctx1 to 15
fin bar-2
set top for ctx1 to 14
fin bar-1
set top for ctx1 to 13
fin bar-0
set top for ctx1 to 12
fin bar-7
set top for ctx1 to 11
fin bar-6
set top for ctx1 to 10
fin bar-5
set top for ctx1 to 9
fin foo-9
set top for ctx1 to 8
fin foo-8
set top for ctx1 to 7
fin foo-7
set top for ctx1 to 6
fin foo-6
set top for ctx1 to 5
fin foo-5
set top for ctx1 to 4
fin foo-4
set top for ctx1 to 3
fin foo-3
set top for ctx1 to 2
fin foo-2
set top for ctx1 to 1
fin foo-1
set top for ctx1 to 0
fin foo-0
final top: 2
==> rc=0, result='undefined'
*** test_xmove_top_samectx (duk_safe_call)
ctx1 (top=10): foo0 foo1 foo2 foo3 foo4 foo5 foo6 foo7 foo8 foo9
ctx2 (top=8): bar0 bar1 bar2 bar3 bar4 bar5 bar6 bar7
==> rc=1, result='Error: invalid context'
*** test_xmove_top_negcount (duk_safe_call)
ctx1 (top=10): foo0 foo1 foo2 foo3 foo4 foo5 foo6 foo7 foo8 foo9
ctx2 (top=8): bar0 bar1 bar2 bar3 bar4 bar5 bar6 bar7
==> rc=1, result='Error: invalid count'
*** test_xmove_top_verylargecount (duk_safe_call)
ctx1 (top=10): foo0 foo1 foo2 foo3 foo4 foo5 foo6 foo7 foo8 foo9
ctx2 (top=8): bar0 bar1 bar2 bar3 bar4 bar5 bar6 bar7
==> rc=1, result='Error: invalid count'
*** test_xmove_top_notenoughsrc (duk_safe_call)
ctx1 (top=10): foo0 foo1 foo2 foo3 foo4 foo5 foo6 foo7 foo8 foo9
ctx2 (top=8): bar0 bar1 bar2 bar3 bar4 bar5 bar6 bar7
==> rc=1, result='Error: invalid count'
*** test_xmove_top_notenoughdst (duk_safe_call)
ctx1 (top=10): foo0 foo1 foo2 foo3 foo4 foo5 foo6 foo7 foo8 foo9
ctx2 (top=1008): bar0 bar1 bar2 bar3 bar4 bar5 bar6 bar7 str-0 str-1 str-2 str-3 str-4 str-5 str-6 str-7 str-8 str-9 str-10 str-11 str-12 str-13 str-14 str-15 str-16 str-17 str-18 str-19 str-20 str-21 str-22 str-23 str-24 str-25 str-26 str-27 str-28 str-29 str-30 str-31 str-32 str-33 str-34 str-35 str-36 str-37 str-38 str-39 str-40 str-41 str-42 str-43 str-44 str-45 str-46 str-47 str-48 str-49 str-50 str-51 str-52 str-53 str-54 str-55 str-56 str-57 str-58 str-59 str-60 str-61 str-62 str-63 str-64 str-65 str-66 str-67 str-68 str-69 str-70 str-71 str-72 str-73 str-74 str-75 str-76 str-77 str-78 str-79 str-80 str-81 str-82 str-83 str-84 str-85 str-86 str-87 str-88 str-89 str-90 str-91 str-92 str-93 str-94 str-95 str-96 str-97 str-98 str-99 str-100 str-101 str-102 str-103 str-104 str-105 str-106 str-107 str-108 str-109 str-110 str-111 str-112 str-113 str-114 str-115 str-116 str-117 str-118 str-119 str-120 str-121 str-122 str-123 str-124 str-125 str-126 str-127 str-128 str-129 str-130 str-131 str-132 str-133 str-134 str-135 str-136 str-137 str-138 str-139 str-140 str-141 str-142 str-143 str-144 str-145 str-146 str-147 str-148 str-149 str-150 str-151 str-152 str-153 str-154 str-155 str-156 str-157 str-158 str-159 str-160 str-161 str-162 str-163 str-164 str-165 str-166 str-167 str-168 str-169 str-170 str-171 str-172 str-173 str-174 str-175 str-176 str-177 str-178 str-179 str-180 str-181 str-182 str-183 str-184 str-185 str-186 str-187 str-188 str-189 str-190 str-191 str-192 str-193 str-194 str-195 str-196 str-197 str-198 str-199 str-200 str-201 str-202 str-203 str-204 str-205 str-206 str-207 str-208 str-209 str-210 str-211 str-212 str-213 str-214 str-215 str-216 str-217 str-218 str-219 str-220 str-221 str-222 str-223 str-224 str-225 str-226 str-227 str-228 str-229 str-230 str-231 str-232 str-233 str-234 str-235 str-236 str-237 str-238 str-239 str-240 str-241 str-242 str-243 str-244 str-245 str-246 str-247 str-248 str-249 str-250 str-251 str-252 str-253 str-254 str-255 str-256 str-257 str-258 str-259 str-260 str-261 str-262 str-263 str-264 str-265 str-266 str-267 str-268 str-269 str-270 str-271 str-272 str-273 str-274 str-275 str-276 str-277 str-278 str-279 str-280 str-281 str-282 str-283 str-284 str-285 str-286 str-287 str-288 str-289 str-290 str-291 str-292 str-293 str-294 str-295 str-296 str-297 str-298 str-299 str-300 str-301 str-302 str-303 str-304 str-305 str-306 str-307 str-308 str-309 str-310 str-311 str-312 str-313 str-314 str-315 str-316 str-317 str-318 str-319 str-320 str-321 str-322 str-323 str-324 str-325 str-326 str-327 str-328 str-329 str-330 str-331 str-332 str-333 str-334 str-335 str-336 str-337 str-338 str-339 str-340 str-341 str-342 str-343 str-344 str-345 str-346 str-347 str-348 str-349 str-350 str-351 str-352 str-353 str-354 str-355 str-356 str-357 str-358 str-359 str-360 str-361 str-362 str-363 str-364 str-365 str-366 str-367 str-368 str-369 str-370 str-371 str-372 str-373 str-374 str-375 str-376 str-377 str-378 str-379 str-380 str-381 str-382 str-383 str-384 str-385 str-386 str-387 str-388 str-389 str-390 str-391 str-392 str-393 str-394 str-395 str-396 str-397 str-398 str-399 str-400 str-401 str-402 str-403 str-404 str-405 str-406 str-407 str-408 str-409 str-410 str-411 str-412 str-413 str-414 str-415 str-416 str-417 str-418 str-419 str-420 str-421 str-422 str-423 str-424 str-425 str-426 str-427 str-428 str-429 str-430 str-431 str-432 str-433 str-434 str-435 str-436 str-437 str-438 str-439 str-440 str-441 str-442 str-443 str-444 str-445 str-446 str-447 str-448 str-449 str-450 str-451 str-452 str-453 str-454 str-455 str-456 str-457 str-458 str-459 str-460 str-461 str-462 str-463 str-464 str-465 str-466 str-467 str-468 str-469 str-470 str-471 str-472 str-473 str-474 str-475 str-476 str-477 str-478 str-479 str-480 str-481 str-482 str-483 str-484 str-485 str-486 str-487 str-488 str-489 str-490 str-491 str-492 str-493 str-494 str-495 str-496 str-497 str-498 str-499 str-500 str-501 str-502 str-503 str-504 str-505 str-506 str-507 str-508 str-509 str-510 str-511 str-512 str-513 str-514 str-515 str-516 str-517 str-518 str-519 str-520 str-521 str-522 str-523 str-524 str-525 str-526 str-527 str-528 str-529 str-530 str-531 str-532 str-533 str-534 str-535 str-536 str-537 str-538 str-539 str-540 str-541 str-542 str-543 str-544 str-545 str-546 str-547 str-548 str-549 str-550 str-551 str-552 str-553 str-554 str-555 str-556 str-557 str-558 str-559 str-560 str-561 str-562 str-563 str-564 str-565 str-566 str-567 str-568 str-569 str-570 str-571 str-572 str-573 str-574 str-575 str-576 str-577 str-578 str-579 str-580 str-581 str-582 str-583 str-584 str-585 str-586 str-587 str-588 str-589 str-590 str-591 str-592 str-593 str-594 str-595 str-596 str-597 str-598 str-599 str-600 str-601 str-602 str-603 str-604 str-605 str-606 str-607 str-608 str-609 str-610 str-611 str-612 str-613 str-614 str-615 str-616 str-617 str-618 str-619 str-620 str-621 str-622 str-623 str-624 str-625 str-626 str-627 str-628 str-629 str-630 str-631 str-632 str-633 str-634 str-635 str-636 str-637 str-638 str-639 str-640 str-641 str-642 str-643 str-644 str-645 str-646 str-647 str-648 str-649 str-650 str-651 str-652 str-653 str-654 str-655 str-656 str-657 str-658 str-659 str-660 str-661 str-662 str-663 str-664 str-665 str-666 str-667 str-668 str-669 str-670 str-671 str-672 str-673 str-674 str-675 str-676 str-677 str-678 str-679 str-680 str-681 str-682 str-683 str-684 str-685 str-686 str-687 str-688 str-689 str-690 str-691 str-692 str-693 str-694 str-695 str-696 str-697 str-698 str-699 str-700 str-701 str-702 str-703 str-704 str-705 str-706 str-707 str-708 str-709 str-710 str-711 str-712 str-713 str-714 str-715 str-716 str-717 str-718 str-719 str-720 str-721 str-722 str-723 str-724 str-725 str-726 str-727 str-728 str-729 str-730 str-731 str-732 str-733 str-734 str-735 str-736 str-737 str-738 str-739 str-740 str-741 str-742 str-743 str-744 str-745 str-746 str-747 str-748 str-749 str-750 str-751 str-752 str-753 str-754 str-755 str-756 str-757 str-758 str-759 str-760 str-761 str-762 str-763 str-764 str-765 str-766 str-767 str-768 str-769 str-770 str-771 str-772 str-773 str-774 str-775 str-776 str-777 str-778 str-779 str-780 str-781 str-782 str-783 str-784 str-785 str-786 str-787 str-788 str-789 str-790 str-791 str-792 str-793 str-794 str-795 str-796 str-797 str-798 str-799 str-800 str-801 str-802 str-803 str-804 str-805 str-806 str-807 str-808 str-809 str-810 str-811 str-812 str-813 str-814 str-815 str-816 str-817 str-818 str-819 str-820 str-821 str-822 str-823 str-824 str-825 str-826 str-827 str-828 str-829 str-830 str-831 str-832 str-833 str-834 str-835 str-836 str-837 str-838 str-839 str-840 str-841 str-842 str-843 str-844 str-845 str-846 str-847 str-848 str-849 str-850 str-851 str-852 str-853 str-854 str-855 str-856 str-857 str-858 str-859 str-860 str-861 str-862 str-863 str-864 str-865 str-866 str-867 str-868 str-869 str-870 str-871 str-872 str-873 str-874 str-875 str-876 str-877 str-878 str-879 str-880 str-881 str-882 str-883 str-884 str-885 str-886 str-887 str-888 str-889 str-890 str-891 str-892 str-893 str-894 str-895 str-896 str-897 str-898 str-899 str-900 str-901 str-902 str-903 str-904 str-905 str-906 str-907 str-908 str-909 str-910 str-911 str-912 str-913 str-914 str-915 str-916 str-917 str-918 str-919 str-920 str-921 str-922 str-923 str-924 str-925 str-926 str-927 str-928 str-929 str-930 str-931 str-932 str-933 str-934 str-935 str-936 str-937 str-938 str-939 str-940 str-941 str-942 str-943 str-944 str-945 str-946 str-947 str-948 str-949 str-950 str-951 str-952 str-953 str-954 str-955 str-956 str-957 str-958 str-959 str-960 str-961 str-962 str-963 str-964 str-965 str-966 str-967 str-968 str-969 str-970 str-971 str-972 str-973 str-974 str-975 str-976 str-977 str-978 str-979 str-980 str-981 str-982 str-983 str-984 str-985 str-986 str-987 str-988 str-989 str-990 str-991 str-992 str-993 str-994 str-995 str-996 str-997 str-998 str-999
==> rc=1, result='Error: attempt to push beyond currently allocated stack'
still here
===*/

static void prep_plain(duk_context *ctx, duk_context **out1, duk_context **out2) {
	duk_context *ctx1, *ctx2;
	int i;

	/* Test contexts: one with shared globals, one with new globals
	 * (although this should make no difference).
	 *
	 * Use a different element count so that if the implementation
	 * checks limits against the wrong stack, it's easier to catch.
	 */

	duk_set_top(ctx, 0);
	duk_push_thread(ctx);
	duk_push_thread_new_globalenv(ctx);
	ctx1 = duk_require_context(ctx, -2);
	ctx2 = duk_require_context(ctx, -1);

	for (i = 0; i < 10; i++) {
		duk_push_sprintf(ctx1, "foo%d", i);
	}
	for (i = 0; i < 8; i++) {
		duk_push_sprintf(ctx2, "bar%d", i);
	}

	*out1 = ctx1;
	*out2 = ctx2;
}

static void prep_large(duk_context *ctx, duk_context **out1, duk_context **out2) {
	duk_context *ctx1, *ctx2;
	int i;

	duk_set_top(ctx, 0);
	duk_push_thread(ctx);
	duk_push_thread_new_globalenv(ctx);
	ctx1 = duk_require_context(ctx, -2);
	ctx2 = duk_require_context(ctx, -1);

	duk_require_stack(ctx1, 100000);
	duk_require_stack(ctx2, 80000);

	for (i = 0; i < 100000; i++) {
		duk_push_sprintf(ctx1, "foo%d", i);
	}
	for (i = 0; i < 80000; i++) {
		duk_push_sprintf(ctx2, "bar%d", i);
	}

	*out1 = ctx1;
	*out2 = ctx2;
}

static void prep_finalizers(duk_context *ctx, duk_context **out1, duk_context **out2) {
	duk_context *ctx1, *ctx2;
	int i;

	duk_set_top(ctx, 0);
	duk_push_thread(ctx);
	duk_push_thread_new_globalenv(ctx);
	ctx1 = duk_require_context(ctx, -2);
	ctx2 = duk_require_context(ctx, -1);

	/* Use a global finalizer to avoid any chance of the object being finalized
	 * residing in the closure of the finalizer and thus participating in a
	 * reference loop.  For instance, this works badly because 'o' remains
	 * reachable from the function closure:
	 *
	 *    "(function(){ var o={};"
	 *    "             Duktape.fin(o, function() { print('fin foo-%d'); });"
	 *    "             return o; })();", i);
	 */

	duk_eval_string_noresult(ctx1, "globalFinalizer = function (o) { print('fin', o.name); };");
	duk_eval_string_noresult(ctx1, "globalToString = function () { return '[' + this.name + ']'; };");
	duk_eval_string_noresult(ctx2, "globalFinalizer = function (o) { print('fin', o.name); };");
	duk_eval_string_noresult(ctx2, "globalToString = function () { return '[' + this.name + ']'; };");

	for (i = 0; i < 10; i++) {
		duk_push_sprintf(ctx1,
		    "(function(){ var o={ name: 'foo-%d' };"
		    "             Duktape.fin(o, globalFinalizer);"
		    "             o.toString = globalToString;"
		    "             return o; })();", i);
		duk_eval(ctx1);
	}
	for (i = 0; i < 8; i++) {
		duk_push_sprintf(ctx2,
		    "(function(){ var o={ name: 'bar-%d' };"
		    "             Duktape.fin(o, globalFinalizer);"
		    "             o.toString = globalToString;"
		    "             return o; })();", i);
		duk_eval(ctx2);
	}

	*out1 = ctx1;
	*out2 = ctx2;
}

static void dump_stack(duk_context *ctx, const char *name) {
	duk_idx_t i, n;

	n = duk_get_top(ctx);
	printf("%s (top=%ld):", name, (long) n);
	for (i = 0; i < n; i++) {
		printf(" ");
		duk_dup(ctx, i);
		printf("%s", duk_safe_to_string(ctx, -1));
		duk_pop(ctx);
	}
	printf("\n");
	fflush(stdout);
}

static void unwind_logged(duk_context *ctx, const char *name) {
	while (duk_get_top(ctx) > 0) {
		duk_idx_t new_top = duk_get_top(ctx) - 1;
		printf("set top for %s to %ld\n", name, (long) new_top);
		fflush(stdout);
		duk_set_top(ctx, new_top);
#if 0
		printf("explicit gc\n");
		fflush(stdout);
		duk_gc(ctx, 0);
#endif
	}
}

static duk_ret_t test_xcopy_top_basic(duk_context *ctx) {
	duk_context *ctx1, *ctx2;

	prep_plain(ctx, &ctx1, &ctx2);

	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xcopy_top(ctx1, ctx2, 0);
	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xcopy_top(ctx1, ctx2, 3);
	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xcopy_top(ctx1, ctx2, 8);  /* exact allowed limit */
	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	fflush(stdout);
	return 0;
}

static duk_ret_t test_xcopy_top_large(duk_context *ctx) {
	duk_context *ctx1, *ctx2;

	prep_large(ctx, &ctx1, &ctx2);

	duk_require_stack(ctx1, 80000);
	duk_xcopy_top(ctx1, ctx2, 80000);
	printf("ctx1 top: %ld\n", (long) duk_get_top(ctx1));
	printf("ctx2 top: %ld\n", (long) duk_get_top(ctx2));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	fflush(stdout);
	return 0;
}

static duk_ret_t test_xcopy_top_refcount(duk_context *ctx) {
	duk_context *ctx1, *ctx2;

	prep_finalizers(ctx, &ctx1, &ctx2);

	/* Test that refcounts work properly, i.e. finalizers are executed
	 * in the expected spots.  This won't work reliably with mark-and-sweep
	 * only.
	 */

	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xcopy_top(ctx1, ctx2, 0);
	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xcopy_top(ctx1, ctx2, 3);
	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xcopy_top(ctx1, ctx2, 8);  /* exact allowed limit */
	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");

	/* Nothing gets finalized here because all refs are in ctx2 */
	unwind_logged(ctx2, "ctx2");

	/* Here finalizers run, one by one */
	unwind_logged(ctx1, "ctx1");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	fflush(stdout);
	return 0;
}

static duk_ret_t test_xcopy_top_samectx(duk_context *ctx) {
	duk_context *ctx1, *ctx2;

	prep_plain(ctx, &ctx1, &ctx2);

	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xcopy_top(ctx1, ctx1, 5);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	fflush(stdout);
	return 0;
}

static duk_ret_t test_xcopy_top_negcount(duk_context *ctx) {
	duk_context *ctx1, *ctx2;

	prep_plain(ctx, &ctx1, &ctx2);

	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xcopy_top(ctx1, ctx2, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	fflush(stdout);
	return 0;
}

static duk_ret_t test_xcopy_top_verylargecount(duk_context *ctx) {
	duk_context *ctx1, *ctx2;

	prep_plain(ctx, &ctx1, &ctx2);

	/* The 'count' will exceed valstack_max and hit a different check than
	 * exceeding the source stack count by a little.
	 */

	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xcopy_top(ctx1, ctx2, DUK_IDX_MAX);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	fflush(stdout);
	return 0;
}

static duk_ret_t test_xcopy_top_notenoughsrc(duk_context *ctx) {
	duk_context *ctx1, *ctx2;

	prep_plain(ctx, &ctx1, &ctx2);

	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xcopy_top(ctx1, ctx2, 8 + 1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	fflush(stdout);
	return 0;
}

static duk_ret_t test_xcopy_top_notenoughdst(duk_context *ctx) {
	duk_context *ctx1, *ctx2;
	int i;

	prep_plain(ctx, &ctx1, &ctx2);
	duk_require_stack(ctx2, 1000);
	for (i = 0; i < 1000; i++) {
		duk_push_sprintf(ctx2, "str-%d", i);
	}

	/* Copy so many elements that the 'required' size for the target
	 * stack is exceeded.
	 */
	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xcopy_top(ctx1, ctx2, 1000);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	fflush(stdout);
	return 0;
}

static duk_ret_t test_xmove_top_basic(duk_context *ctx) {
	duk_context *ctx1, *ctx2;

	prep_plain(ctx, &ctx1, &ctx2);

	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xmove_top(ctx1, ctx2, 3);
	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xmove_top(ctx1, ctx2, 5);  /* exact allowed limit */
	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	fflush(stdout);
	return 0;
}

static duk_ret_t test_xmove_top_large(duk_context *ctx) {
	duk_context *ctx1, *ctx2;

	prep_large(ctx, &ctx1, &ctx2);

	duk_require_stack(ctx1, 80000);
	duk_xmove_top(ctx1, ctx2, 80000);
	printf("ctx1 top: %ld\n", (long) duk_get_top(ctx1));
	printf("ctx2 top: %ld\n", (long) duk_get_top(ctx2));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	fflush(stdout);
	return 0;
}

static duk_ret_t test_xmove_top_refcount(duk_context *ctx) {
	duk_context *ctx1, *ctx2;

	prep_finalizers(ctx, &ctx1, &ctx2);

	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xmove_top(ctx1, ctx2, 3);
	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xmove_top(ctx1, ctx2, 5);  /* exact allowed limit */
	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");

	/* Nothing gets finalized here because all refs are in ctx2 */
	unwind_logged(ctx2, "ctx2");

	/* Here finalizers run, one by one */
	unwind_logged(ctx1, "ctx1");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	fflush(stdout);
	return 0;
}

static duk_ret_t test_xmove_top_samectx(duk_context *ctx) {
	duk_context *ctx1, *ctx2;

	prep_plain(ctx, &ctx1, &ctx2);

	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xmove_top(ctx1, ctx1, 5);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	fflush(stdout);
	return 0;
}

static duk_ret_t test_xmove_top_negcount(duk_context *ctx) {
	duk_context *ctx1, *ctx2;

	prep_plain(ctx, &ctx1, &ctx2);

	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xmove_top(ctx1, ctx2, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	fflush(stdout);
	return 0;
}

static duk_ret_t test_xmove_top_verylargecount(duk_context *ctx) {
	duk_context *ctx1, *ctx2;

	prep_plain(ctx, &ctx1, &ctx2);

	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xmove_top(ctx1, ctx2, DUK_IDX_MAX);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	fflush(stdout);
	return 0;
}

static duk_ret_t test_xmove_top_notenoughsrc(duk_context *ctx) {
	duk_context *ctx1, *ctx2;

	prep_plain(ctx, &ctx1, &ctx2);

	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xmove_top(ctx1, ctx2, 8 + 1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	fflush(stdout);
	return 0;
}

static duk_ret_t test_xmove_top_notenoughdst(duk_context *ctx) {
	duk_context *ctx1, *ctx2;
	int i;

	prep_plain(ctx, &ctx1, &ctx2);
	duk_require_stack(ctx2, 1000);
	for (i = 0; i < 1000; i++) {
		duk_push_sprintf(ctx2, "str-%d", i);
	}

	dump_stack(ctx1, "ctx1");
	dump_stack(ctx2, "ctx2");
	duk_xmove_top(ctx1, ctx2, 1000);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	fflush(stdout);
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_xcopy_top_basic);
	TEST_SAFE_CALL(test_xcopy_top_large);
	TEST_SAFE_CALL(test_xcopy_top_refcount);
	TEST_SAFE_CALL(test_xcopy_top_samectx);
	TEST_SAFE_CALL(test_xcopy_top_negcount);
	TEST_SAFE_CALL(test_xcopy_top_verylargecount);
	TEST_SAFE_CALL(test_xcopy_top_notenoughsrc);
	TEST_SAFE_CALL(test_xcopy_top_notenoughdst);

	TEST_SAFE_CALL(test_xmove_top_basic);
	TEST_SAFE_CALL(test_xmove_top_large);
	TEST_SAFE_CALL(test_xmove_top_refcount);
	TEST_SAFE_CALL(test_xmove_top_samectx);
	TEST_SAFE_CALL(test_xmove_top_negcount);
	TEST_SAFE_CALL(test_xmove_top_verylargecount);
	TEST_SAFE_CALL(test_xmove_top_notenoughsrc);
	TEST_SAFE_CALL(test_xmove_top_notenoughdst);

	printf("still here\n");
	fflush(stdout);
}
