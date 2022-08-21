/*
 *  Prime search based on web workers example.
 *
 *  http://www.w3.org/TR/workers/
 */

/*===
2
3
5
7
11
13
17
19
23
29
31
37
41
43
47
53
59
61
67
71
73
79
83
89
97
===*/

var n = 1;
search: while (n < 100) {
    n++;

    for (var i = 2; i <= Math.sqrt(n); i += 1)
        if (n % i == 0)
            continue search;

     print(n);
}
