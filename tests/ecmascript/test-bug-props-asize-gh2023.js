/*
 *  https://github.com/svaarala/duktape/issues/2023
 */

/*===
===*/

function mapchar ( v ) { }
var input ;
var round ;
input = [ ] ;
input[ 65536 ] = 0 ;
input.map( mapchar ).join( '' );
