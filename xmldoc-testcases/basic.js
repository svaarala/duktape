var xmldata =
	'<test>' +
	'<foo attr="123">' +
	'Content &amp; stuff' +
	'</foo>' +
	'<foo attr="124">' +
	'Second child' +
	'</foo>' +
	'<foo attr="125">' +
	'Third child' +
	'</foo>' +
	'</test>';

print('=== input xml data')
print(xmldata);

print('=== parsing')
var doc = new XmlDocument(xmldata)
print(typeof doc);

print('=== pretty printed')
print(doc)

print('=== loop over "foo" children')
doc.eachChild(function (x) {
    print(Duktape.enc('jx', x));
    print(x);
});
