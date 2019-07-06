/*
 *  https://github.com/svaarala/duktape/issues/2023
 */

/*===
done
===*/

function mapchar ( v ) { }
var input ;
var round ;
input = [ ] ;
input[ 65536 ] = 0 ;
input.map( mapchar ).join( '' );

print('done');
