// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// Test case for XML ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: XML.as,v 1.54 2007/12/21 17:29:51 strk Exp $";

#include "check.as"
//#include "dejagnu.as"
#include "utils.as"

var existtests = true;

check(XML);

#if OUTPUT_VERSION >= 6 // {

check(! XML.prototype.hasOwnProperty("appendChild") );
check(! XML.prototype.hasOwnProperty("cloneNode") );
check(! XML.prototype.hasOwnProperty("hasChildNodes") );
check(! XML.prototype.hasOwnProperty("insertBefore") );
check(! XML.prototype.hasOwnProperty("removeNode") );
check(! XML.prototype.hasOwnProperty("cloneNode") );
check(! XML.prototype.hasOwnProperty("toString") );
check(! XML.prototype.hasOwnProperty("length") );
check(! XML.prototype.hasOwnProperty("status"));
check(! XML.prototype.hasOwnProperty("loaded"));
check(! XML.prototype.hasOwnProperty("attributes"));
check(! XML.prototype.hasOwnProperty("nodeValue"));
check(XML.prototype.hasOwnProperty("onData"));
xcheck(XML.prototype.hasOwnProperty("onLoad")); // it seems it wouldn't do anything anyway, would it ?
check(XML.prototype.hasOwnProperty("createElement") );
check(XML.prototype.hasOwnProperty("addRequestHeader") );
check(XML.prototype.hasOwnProperty("createTextNode") );
check(XML.prototype.hasOwnProperty("getBytesLoaded") );
check(XML.prototype.hasOwnProperty("getBytesTotal") );
check(XML.prototype.hasOwnProperty("load") );
check(XML.prototype.hasOwnProperty("parseXML") );
check(XML.prototype.hasOwnProperty("send") );
check(XML.prototype.hasOwnProperty("sendAndLoad") );

check(!XML.hasOwnProperty("createElement") );
check(!XML.hasOwnProperty("addRequestHeader") );
check(!XML.hasOwnProperty("createTextNode") );
check(!XML.hasOwnProperty("getBytesLoaded") );
check(!XML.hasOwnProperty("getBytesTotal") );
check(!XML.hasOwnProperty("load") );
check(!XML.hasOwnProperty("parseXML") );
check(!XML.hasOwnProperty("send") );
check(!XML.hasOwnProperty("sendAndLoad") );
check(!XML.hasOwnProperty("nodeValue"));
// ignoreWhite is undefined by default, but is used when set to true
check(!XML.prototype.hasOwnProperty("ignoreWhite") );

check(XMLNode.prototype.hasOwnProperty("appendChild") );
check(XMLNode.prototype.hasOwnProperty("cloneNode") );
check(XMLNode.prototype.hasOwnProperty("hasChildNodes") );
check(XMLNode.prototype.hasOwnProperty("insertBefore") );
check(XMLNode.prototype.hasOwnProperty("removeNode") );
check(XMLNode.prototype.hasOwnProperty("toString") );
check(XMLNode.prototype.hasOwnProperty("cloneNode") );
check(XMLNode.prototype.hasOwnProperty("attributes") );
check(XMLNode.prototype.hasOwnProperty("parentNode") );
check(XMLNode.prototype.hasOwnProperty("nodeValue"));
check(! XMLNode.prototype.hasOwnProperty("onData") );
check(! XMLNode.prototype.hasOwnProperty("length") );
check(! XMLNode.prototype.hasOwnProperty("createElement") );
check(! XMLNode.prototype.hasOwnProperty("addRequestHeader") );
check(! XMLNode.prototype.hasOwnProperty("createTextNode") );
check(! XMLNode.prototype.hasOwnProperty("getBytesLoaded") );
check(! XMLNode.prototype.hasOwnProperty("getBytesTotal") );
check(! XMLNode.prototype.hasOwnProperty("load") );
check(! XMLNode.prototype.hasOwnProperty("parseXML") );
check(! XMLNode.prototype.hasOwnProperty("send") );
check(! XMLNode.prototype.hasOwnProperty("sendAndLoad") );
check(! XMLNode.prototype.hasOwnProperty("status"));
check(! XMLNode.prototype.hasOwnProperty("loaded"));

check(! XMLNode.hasOwnProperty("appendChild") );
check(! XMLNode.hasOwnProperty("cloneNode") );
check(! XMLNode.hasOwnProperty("hasChildNodes") );
check(! XMLNode.hasOwnProperty("insertBefore") );
check(! XMLNode.hasOwnProperty("removeNode") );
check(! XMLNode.hasOwnProperty("toString") );
check(! XMLNode.hasOwnProperty("cloneNode") );
check(! XMLNode.hasOwnProperty("nodeValue"));

#endif // OUTPUT_VERSION >= 6 }

check(XML.prototype instanceof XMLNode);

var tmp = new XML();

#if OUTPUT_VERSION >= 6
 check( ! tmp.hasOwnProperty("nodeValue") );
#endif

check(tmp instanceof XML);
check(tmp instanceof XMLNode);

check_equals(typeof(tmp.length), 'undefined');
check(! tmp.hasOwnProperty("length"));

check_equals(typeof(tmp.status), 'number');
check(! tmp.hasOwnProperty("status"));
#if OUTPUT_VERSION < 6
 check(! tmp.__proto__.hasOwnProperty('status') );
#else
 xcheck(tmp.__proto__.hasOwnProperty('status') );
#endif

check_equals(tmp.status, 0);
tmp.status = -1;
check_equals(tmp.status, -1);
tmp.status = 1000;
check_equals(tmp.status, 1000);
o = new Object;
tmp.status = o; 
check_equals(typeof(tmp.status), 'number');
check_equals(tmp.status, -2147483648.0); // 0xFFFFFFFF
tmp.status = 7;
returnFour = function() { return 4; };
o.toString = returnFour;
tmp.status = o;
check_equals(typeof(tmp.status), 'number');
check_equals(tmp.status, -2147483648.0); // 0xFFFFFFFF

check_equals(typeof(tmp.loaded), 'undefined');
check(! tmp.hasOwnProperty("loaded"));

tmp.loaded = 5;
check_equals(typeof(tmp.loaded), 'boolean');
check(tmp.loaded);
tmp.loaded = 0;
check_equals(typeof(tmp.loaded), 'boolean');
check(!tmp.loaded);
check(! tmp.hasOwnProperty("loaded"));


// test the XML constuctor
if (tmp) {
    pass("XML::XML() constructor");
} else {
    fail("XML::XML()");		
}

check(XML);

if (existtests) {
    
    // test the XML::addrequestheader method
    if (tmp.addRequestHeader) {
        pass("XML::addRequestHeader() exists");
    } else {
        fail("XML::addRequestHeader() doesn't exist");
    }
// test the XML::appendchild method
    if (tmp.appendChild) {
	pass("XML::appendChild() exists");
    } else {
	fail("XML::appendChild() doesn't exist");
    }
// test the XML::clonenode method
    if (tmp.cloneNode) {
	pass("XML::cloneNode() exists");
    } else {
	fail("XML::cloneNode() doesn't exist");
    }
// test the XML::createelement method
    if (tmp.createElement) {
	pass("XML::createElement() exists");
    } else {
	fail("XML::createElement() doesn't exist");
    }
// test the XML::createtextnode method
    if (tmp.createTextNode) {
	pass("XML::createTextNode() exists");
    } else {
	fail("XML::createTextNode() doesn't exist");
    }
// test the XML::getbytesloaded method
    if (tmp.getBytesLoaded) {
        pass("XML::getBytesLoaded() exists");
    } else {
        fail("XML::getBytesLoaded() doesn't exist");
    }
// test the XML::getbytestotal method
    if (tmp.getBytesTotal) {
	pass("XML::getBytesTotal() exists");
    } else {
	fail("XML::getBytesTotal() doesn't exist");
    }
// test the XML::haschildnodes method
    if (tmp.hasChildNodes) {
	pass("XML::hasChildNodes() exists");
    } else {
	fail("XML::hasChildNodes() doesn't exist");
    }
// test the XML::insertbefore method
    if (tmp.insertBefore) {
	pass("XML::insertBefore() exists");
    } else {
	fail("XML::insertBefore() doesn't exist");
    }
// test the XML::load method
    if (tmp.load) {
	pass("XML::load() exists");
    } else {
	fail("XML::load() doesn't exist");
    }
// This doesn't seem to exist in the real player
// test the XML::loaded method
    if (tmp.loaded) {
	unresolved("XML::loaded() exists, it shouldn't!");
    } else {
	unresolved("XML::loaded() doesn't exist yet");
    }
    
//test the XML::parse method
    if (tmp.parseXML) {
	pass("XML::parseXML() exists");
    } else {
	fail("XML::parseXML() doesn't exist");
    }
// test the XML::removenode method
    if (tmp.removeNode) {
	pass("XML::removeNode() exists");
    } else {
	fail("XML::removeNode() doesn't exist");
    }
// test the XML::send method
    if (tmp.send) {
	pass("XML::send() exists");
    } else {
	fail("XML::send() doesn't exist");
    }
// test the XML::sendandload method
    if (tmp.sendAndLoad) {
	pass("XML::sendAndLoad() exists");
    } else {
	fail("XML::sendAndLoad() doesn't exist");
    }
// test the XML::tostring method
    if (tmp.toString) {
	pass("XML::toString() exists");
    } else {
	fail("XML::toString() doesn't exist");
    }
    
} // end of existtests
/////////////////////////////////////////////////////

check(XML);
newXML = new XML();
check(XML);

// Load
// if (tmp.load("testin.xml")) {
// 	pass("XML::load() works");
// } else {
// 	fail("XML::load() doesn't work");
// }
check(XML);

// Use escaped " instead of ' so that it matches xml_out (comments and CDATA tags stripped).
var xml_in = "<TOPNODE tna1=\"tna1val\" tna2=\"tna2val\" tna3=\"tna3val\"><SUBNODE1 sna1=\"sna1val\" sna2=\"sna2val\"><SUBSUBNODE1 ssna1=\"ssna1val\" ssna2=\"ssna2val\"><!-- comment should be ignored-->sub sub1 node data 1</SUBSUBNODE1><SUBSUBNODE2><!--comment: cdata with illegal characters --><![CDATA[sub /\sub1 <br>\"node data 2\"]]></SUBSUBNODE2></SUBNODE1><SUBNODE2><SUBSUBNODE1>sub sub2 node data 1</SUBSUBNODE1><SUBSUBNODE2>sub sub2 node data 2</SUBSUBNODE2></SUBNODE2></TOPNODE>";
// with comments stripped out.
var xml_out = '<TOPNODE tna1="tna1val" tna2="tna2val" tna3="tna3val"><SUBNODE1 sna1="sna1val" sna2="sna2val"><SUBSUBNODE1 ssna1="ssna1val" ssna2="ssna2val">sub sub1 node data 1</SUBSUBNODE1><SUBSUBNODE2>sub /sub1 &lt;br&gt;&quot;node data 2&quot;</SUBSUBNODE2></SUBNODE1><SUBNODE2><SUBSUBNODE1>sub sub2 node data 1</SUBSUBNODE1><SUBSUBNODE2>sub sub2 node data 2</SUBSUBNODE2></SUBNODE2></TOPNODE>';
//var xml_out = "<TOPNODE tna1=\"tna1val\" tna2=\"tna2val\" tna3=\"tna3val\"><SUBNODE1 sna1=\"sna1val\" sna2=\"sna2val\"><SUBSUBNODE1 ssna1=\"ssna1val\" ssna2=\"ssna2val\">sub sub1 node data 1</SUBSUBNODE1><SUBSUBNODE2>sub /\sub1 \<br\>\"node data 2\"</SUBSUBNODE2></SUBNODE1><SUBNODE2><SUBSUBNODE1>sub sub2 node data 1</SUBSUBNODE1><SUBSUBNODE2>sub sub2 node data 2</SUBSUBNODE2></SUBNODE2></TOPNODE>";


check(XML);
tmp.checkParsed = function ()
{
	note("tmp.checkParsed called");

	// Since we didn't *load* the XML, but we
	// just *parsed* it, expect getBytesLoaded 
	// and getBytesTotal to return undefined
	check_equals(this.getBytesLoaded(), undefined);
	check_equals(this.getBytesTotal(), undefined);

	check(this.hasChildNodes());
	check_equals(typeof(this.nodeName), 'null');
	check_equals(typeof(this.nodeValue), 'null');
	check(this.childNodes instanceof Array);
	check_equals(this.firstChild, this.lastChild);
	check(this.firstChild instanceof XMLNode);
	check_equals(typeof(this.length), 'undefined');
	check_equals(typeof(this.childNodes.length), 'number');
	check_equals(this.childNodes.length, 1);
	check_equals(this.childNodes[0], this.firstChild);
	check_equals(this.childNodes[0], this.lastChild);

#if OUTPUT_VERSION > 5
	check(this.childNodes.hasOwnProperty('length'));
	check(this.childNodes[0] === this.firstChild);
	check(this.childNodes[0] === this.lastChild);
#endif

	// childNodes is a read-only property !
	this.childNodes = 5;
	check(this.childNodes instanceof Array);

        with (this.firstChild)
	{
		check_equals(nodeName, 'TOPNODE');
		check_equals(typeof(nodeValue), 'null');
		check_equals(typeof(nextSibling), 'null');
		check_equals(typeof(previousSibling), 'null');

		// Check attributes
		check_equals(typeof(attributes), 'object');
		check_equals(attributes.tna1, 'tna1val');
		check_equals(attributes.tna2, 'tna2val');
		// Check attributes enumerability
		var attrcopy = {};
		for (var i in attributes) attrcopy[i] = attributes[i];
		check_equals(attrcopy.tna1, 'tna1val');
		check_equals(attrcopy.tna2, 'tna2val');
		

		// Check that nodeValue is overridable
		nodeValue = 4;
		check_equals(typeof(nodeValue), 'string');
		check_equals(nodeValue, '4');

		check_equals(nodeType, 1); // element
		check_equals(childNodes.length, 2);

		with (firstChild)
		{
			check_equals(nodeName, 'SUBNODE1');
			check_equals(typeof(nodeValue), 'null');

			check_equals(typeof(nextSibling), 'object');
			check_equals(nextSibling.nodeName, 'SUBNODE2');
			check_equals(typeof(previousSibling), 'null');

			check_equals(typeof(attributes), 'object');
			check_equals(attributes.sna1, 'sna1val');
			check_equals(attributes.sna2, 'sna2val');

			check_equals(nodeType, 1); // element
			check_equals(childNodes.length, 2);
			with (firstChild)
			{
				check_equals(nodeName, 'SUBSUBNODE1');
				check_equals(typeof(nodeValue), 'null');
				check_equals(nodeType, 1); // element
				check_equals(typeof(nextSibling), 'object');
				check_equals(nextSibling.nodeName, 'SUBSUBNODE2');
				check_equals(typeof(previousSibling), 'null');
				check_equals(typeof(attributes), 'object');
				check_equals(attributes.ssna1, 'ssna1val');
				check_equals(attributes.ssna2, 'ssna2val');
				check_equals(childNodes.length, 1);
				with (firstChild)
				{
					check_equals(typeof(nodeName), 'null')
					check_equals(nodeValue, 'sub sub1 node data 1')
					check_equals(nodeType, 3); // text
				}
			}
			with (lastChild)
			{
				check_equals(nodeName, 'SUBSUBNODE2');
				check_equals(typeof(nodeValue), 'null');
				check_equals(nodeType, 1); // element
				check_equals(typeof(nextSibling), 'null');
				check_equals(typeof(previousSibling), 'object');
				check_equals(previousSibling.nodeName, 'SUBSUBNODE1');
				check_equals(childNodes.length, 1);
				with (firstChild)
				{
					check_equals(typeof(nodeName), 'null')
					check_equals(nodeValue, 'sub /\sub1 <br>"node data 2"')
					check_equals(nodeType, 3); // text
				}
			}
		}

		with (lastChild)
		{
			check_equals(nodeName, 'SUBNODE2');
			check_equals(typeof(nodeValue), 'null');
			check_equals(nodeType, 1); // element

			check_equals(typeof(nextSibling), 'null');
			check_equals(typeof(previousSibling), 'object');
			check_equals(previousSibling.nodeName, 'SUBNODE1');

			check_equals(childNodes.length, 2);
			with (firstChild)
			{
				check_equals(nodeName, 'SUBSUBNODE1');
				check_equals(typeof(nodeValue), 'null');
				check_equals(nodeType, 1); // element
				check_equals(childNodes.length, 1);
				with (firstChild)
				{
					check_equals(typeof(nodeName), 'null')
					check_equals(nodeValue, 'sub sub2 node data 1')
					check_equals(nodeType, 3); // text
				}
			}
			with (lastChild)
			{
				check_equals(nodeName, 'SUBSUBNODE2');
				check_equals(typeof(nodeValue), 'null');
				check_equals(nodeType, 1); // element
				check_equals(childNodes.length, 1);
				with (firstChild)
				{
					check_equals(typeof(nodeName), 'null')
					check_equals(nodeValue, 'sub sub2 node data 2')
					check_equals(nodeType, 3); // text
				}
			}
		}
        }
	check_equals(this.firstChild.nodeValue, '4');
};

check_equals( typeof(tmp.parseXML), 'function');
check(tmp.childNodes instanceOf Array);
check_equals(tmp.childNodes.length, 0);

// parseXML doesn't return anything
ret = tmp.parseXML(xml_in);
check_equals(typeof(ret), 'undefined');

tmp.checkParsed(); // onLoad won't be called
//note("Parsed XML: "+tmp.toString());

// TODO: FIX THIS !
xcheck_equals(tmp.toString(), xml_out);

//------------------------------------------------
// Test XML editing
//------------------------------------------------
xml1 = new XML("<X1T><X1C1></X1C1><X1C2></X1C2></X1T>");
xml2 = new XML("<X2T><X2C1></X2C1><X2C2></X2C2></X2T>");
check_equals(xml1.childNodes.length, 1);
check_equals(xml1.firstChild.childNodes.length, 2);
check_equals(xml2.childNodes.length, 1);
check_equals(xml2.firstChild.childNodes.length, 2);

// Now move X1C1 to X2T
movingchild = xml2.firstChild.lastChild;

check_equals(movingchild.parentNode, xml2.firstChild);
check(movingchild.parentNode != xml1.firstChild);
xml1.firstChild.appendChild(movingchild);
check_equals(movingchild.parentNode, xml1.firstChild);
check(movingchild.parentNode != xml2.firstChild);

check_equals(xml1.firstChild.childNodes.length, 3);
check_equals(xml1.firstChild.childNodes[0].nodeName, "X1C1");
check_equals(xml1.firstChild.childNodes[1].nodeName, "X1C2");
check_equals(xml1.firstChild.childNodes[2].nodeName, "X2C2");

check_equals(xml2.firstChild.childNodes.length, 1); // firstChild has been moved
check_equals(xml2.firstChild.childNodes[0].nodeName, "X2C1");

// Now insert X2C1 from xml2 before X1C2 in xml1
xml1.firstChild.insertBefore(
	xml2.firstChild.lastChild, // what to insert
	xml1.firstChild.childNodes[1] // before what to insert it
);
check_equals(xml1.firstChild.childNodes.length, 4);
check_equals(xml1.firstChild.childNodes[0].nodeName, "X1C1");
check_equals(xml1.firstChild.childNodes[1].nodeName, "X2C1");
check_equals(xml1.firstChild.childNodes[2].nodeName, "X1C2");
check_equals(xml1.firstChild.childNodes[3].nodeName, "X2C2");

// XMLNode.removeNode
check_equals(xml2.firstChild.childNodes.length, 0); // lastChild has been moved
xml2.firstChild.removeNode();
check_equals(xml2.childNodes.length, 0); // it's only child has been dropped

x1c1_node = xml1.firstChild.childNodes[0];
x2c1_node = xml1.firstChild.childNodes[1];
check_equals(x1c1_node.nodeName, "X1C1");
check_equals(x1c1_node.nextSibling, x2c1_node);
check_equals(x2c1_node.nodeName, "X2C1");
check_equals(x2c1_node.previousSibling, x1c1_node);
xml1.firstChild.removeNode(); // removeNode removes all childrens !!
check_equals(xml1.childNodes.length, 0);
// so these become orphaned (no parent)
check_equals(x1c1_node.nodeName, "X1C1");
check_equals(x1c1_node.nextSibling(), null);
check_equals(x2c1_node.nodeName, "X2C1");
check_equals(x2c1_node.previousSibling(), null);

xml1.appendChild(x1c1_node);
xml1.appendChild(x2c1_node);
check_equals(x1c1_node.nextSibling, x2c1_node);
check_equals(x2c1_node.previousSibling, x1c1_node);

check_equals(xml1.childNodes.length, 2);
x1c1_node.appendChild(x2c1_node);
check_equals(xml1.childNodes.length, 1);
check_equals(xml1.firstChild.lastChild.nodeName, 'X2C1');

src_node = xml1.firstChild;
cln_node = src_node.cloneNode(); // ! deep
deepcln_node = src_node.cloneNode(true); // deep

check_equals(src_node.childNodes.length, 1);
check_equals(cln_node.childNodes.length, 0); // non-deep clone doesn't clone childs !
check_equals(deepcln_node.childNodes.length, 1);

check_equals(src_node.firstChild.nodeName, 'X2C1');
check_equals(deepcln_node.firstChild.nodeName, 'X2C1');
src_node.firstChild.nodeName = 'X2C1_modified';
check_equals(src_node.firstChild.nodeName, 'X2C1_modified');
check_equals(deepcln_node.firstChild.nodeName, 'X2C1');

check_equals(deepcln_node.parentNode, null);
deepcln_node.parentNode = src_node;
check_equals(deepcln_node.parentNode, null);


xml1 = new XML("<X1T><X1C1><X1C1C1></X1C1C1></X1C1><X1C2></X1C2></X1T>");
check_equals(xml1.firstChild.childNodes.length, 2);
fc = xml1.firstChild.firstChild;
check_equals(fc.nodeName, 'X1C1');
check_equals(fc.childNodes.length, 1);
check_equals(fc.parentNode, xml1.firstChild);
fc.removeNode(); // removes xml1.firstChild.firstChil (X1C1)
check_equals(xml1.firstChild.childNodes.length, 1);
check_equals(fc.parentNode, null);
check_equals(fc.nodeName, 'X1C1');
check_equals(fc.childNodes.length, 1); // childs of the removed child are still alive !
check_equals(fc.childNodes[0].parentNode, fc); // and still point to the removed child !

//------------------------------------------------
// Other tests..
//------------------------------------------------

myXML = new XML();
check(myXML != undefined);
check(myXML.createElement);

//    file.puts("function: dodo()");
// create three XML nodes using createElement()
var element1 = myXML.createElement("element1");
check(element1.nodeName == "element1");

var element2 = myXML.createElement("element2");
check_equals(element2.nodeName, "element2");

var element3 = myXML.createElement("element3");
check_equals(element3.nodeName, "element3");

check(myXML.createTextNode);

// create two XML text nodes using createTextNode()
var textNode1 = myXML.createTextNode("textNode1 String value");
check(textNode1.nodeValue == "textNode1 String value");

var textNode2 = myXML.createTextNode("textNode2 String value");
check(textNode2.nodeValue == "textNode2 String value");

// place the new nodes into the XML tree
check(!element2.hasChildNodes());
ret = element2.appendChild(textNode1);
check_equals(typeof(ret), 'undefined');
check(element2.hasChildNodes());

check_equals(element2.nodeValue, null);
check_equals(typeof(element2.lastChild), 'object');
check_equals(element2.lastChild.nodeValue, "textNode1 String value");
element2.lastChild = 4;
check_equals(typeof(element2.lastChild), 'object');

element3.appendChild(textNode2);
check_equals(element3.nodeValue, null); 
check_equals(typeof(element3.lastChild), 'object');
check_equals(element3.lastChild.nodeValue, "textNode2 String value");

// place the new nodes into the XML tree
doc.appendChild(element1);
//check(doc.firstChild.nodeName == "element1");

element1.appendChild(element2);
check(element1.hasChildNodes());
// trace(element1.nodeName);
// trace(element1.firstChild.nodeName);
check(element1.firstChild.nodeName == "element2");

element2.appendChild(element3);
check(element2.hasChildNodes());

trace(doc.toString());

// // trace(myXML.toString());

// newnode = myXML.cloneNode(false);

// //trace(myXML.nodeName);
// //trace(newnode.nodeValue);

// //trace("Child1" + _global.child1);
// //trace("Child2" + _global.child2);

// // This won't work as onLoad is not called unless you
// // actually *load* the XML, we're using parseXML that
// // does *not* trigger loading (see also getBytesLoaded
// // and getBytesTotal) and does *not* trigger onLoad 
// // event to execute.
// #if 0 
// if ((_global.child1 == "sub sub1 node data 1")
//     && (global.child2 == "sub sub1 node data 2")) {
// 	pass("XML::onLoad works");
// } else {
// 	fail("XML::onLoad doesn't work");
// }
// #endif


//--------------------------------------------------------------------
// Test loading an XML locally
//--------------------------------------------------------------------

myxml = new XML;

check_equals(typeof(myxml.onData), 'function');
#if OUTPUT_VERSION > 5
check(myxml.onData != XML.prototype.parseXML);
#endif

check(!myxml.hasOwnProperty('onLoad'));

myxml.onLoadCalls = 0;

myxml.onLoad = function(success)
{
	note("myxml.onLoad("+success+") called");

	check_equals(typeof(myxml.status), 'number');
	check_equals(typeof(myxml.loaded), 'boolean');
#if OUTPUT_VERSION >= 6
	check(! myxml.hasOwnProperty('status'));
	check(! myxml.hasOwnProperty('loaded'));
#endif // OUTPUT_VERSION >= 6

	if ( ! success )
	{
		note("No success loading gnash.xml");
		check_equals(myxml.status, 0);
		check(! myxml.loaded);
		return;
	}
	note("gnash.xml successfully loaded");
	note("myxml status is "+myxml.status);

	check_equals(myxml.status, 0);
	check(myxml.loaded);

	// Check 'loaded' and 'status' to be "overridable"

	var loaded_backup = myxml.loaded;
	myxml.loaded = 'a string';
	check_equals(typeof(myxml.loaded), 'boolean');
	myxml.loaded = ! loaded_backup;
	check(myxml.loaded != loaded_backup);
	myxml.loaded = loaded_backup;

	var status_backup = myxml.status;
	myxml.status = 'a string';
	check_equals(typeof(myxml.status), 'number');
	check(myxml.status != status_backup);
	note("myxml.status is == "+myxml.status+" after being set to 'a string'");
	myxml.status = status_backup;


	//note("myxml.toString(): "+myxml.toString());
	check_equals(typeof(myxml.attributes), 'object');
	check(! myxml.attributes instanceof Object);
	check_equals(typeof(myxml.attributes.__proto__), 'undefined');

	check(myxml.hasChildNodes());
	check_equals(myxml.nodeName, null);

	topnode = myxml.firstChild;
	check_equals(topnode.nodeName, 'XML');
	check_equals(topnode.attributes.attr1, 'attr1 value');

	// XML, comment, NULL 
	if ( typeof(myxml.lastChildNodesCount) == 'undefined' )
	{
		myxml.lastChildNodesCount = myxml.childNodes.length;
	}
	else
	{
		check_equals(myxml.childNodes.length, myxml.lastChildNodesCount);
	}
	xcheck_equals(myxml.childNodes.length, 3); // gnash fails discarding the comment and the ending blanks

	// We're done
	++this.onLoadCalls;
	note("onLoad called "+this.onLoadCalls+" times");
	if ( this.onLoadCalls == 2 )
	{
#if OUTPUT_VERSION < 6
		check_totals(265);
#else
		check_totals(341);
#endif
		play();
	}

};

check_equals(typeof(myxml.status), 'number');
#if OUTPUT_VERSION < 7
check_equals(typeof(myxml.STATUS), 'number');
#else // OUTPUT_VERSION >= 7
check_equals(typeof(myxml.STATUS), 'undefined');
#endif // OUTPUT_VERSION >= 7
check_equals(typeof(myxml.__proto__.status), 'undefined');
check_equals(typeof(myxml.loaded), 'undefined');
#if OUTPUT_VERSION >= 6
check(!myxml.hasOwnProperty('status'));
check(!myxml.hasOwnProperty('loaded'));
#endif // OUTPUT_VERSION >= 6
ret = myxml.load( MEDIA(gnash.xml) );

check_equals(typeof(myxml.loaded), 'boolean');
#if OUTPUT_VERSION < 7
check_equals(typeof(myxml.LOADED), 'boolean');
#else // OUTPUT_VERSION >= 7
check_equals(typeof(myxml.LOADED), 'undefined');
#endif // OUTPUT_VERSION >= 7
check(! myxml.loaded ); // is really loaded in a background thread

check_equals(myxml.loaded, false ); // is really loaded in a background thread
note("myxml.loaded = "+myxml.loaded);
note("myxml.load() returned "+ret);

// Load again, to verify new parsing doesn't get appended to the old
ret = myxml.load( MEDIA(gnash.xml) );

//------------------------------------------------
// Test XML.ignoreWhite
//------------------------------------------------

myxml2 = new XML();
xmlin = "<X1T> <X1C1> </X1C1> <X1C2>\n</X1C2> 	</X1T>";	 
xmlin2 = "<X0><X1/></X0>";
xmlin2_out = "<X0><X1 /></X0>";
xmlin_stripwhite = "<X1T><X1C1 /><X1C2 /></X1T>";

myxml2.ignoreWhite = false;
myxml2.parseXML(xmlin);
check_equals(myxml2.childNodes.length, 1);  
check_equals(myxml2.toString(), xmlin);  
myxml2.parseXML(xmlin2); // parsing twice doesn't append !
check_equals(myxml2.childNodes.length, 1);  
check_equals(myxml2.toString(), xmlin2_out); 

myxml2.ignoreWhite = true;
myxml2.parseXML(xmlin);
check_equals(myxml2.toString(), xmlin_stripwhite); 

myxml2.ignoreWhite = true;
myxml2.parseXML("<X1> </X1>");
check_equals(myxml2.childNodes.length, 1);
check(!myxml2.childNodes[0].hasChildNodes());
check_equals(myxml2.toString(), "<X1 />"); 

myxml2.ignoreWhite = false;
myxml2.parseXML("<X1> </X1>");
check_equals(myxml2.childNodes.length, 1);
check(myxml2.childNodes[0].hasChildNodes());
check_equals(myxml2.childNodes[0].childNodes[0].nodeType, 3); // text node
check_equals(myxml2.childNodes[0].childNodes[0].nodeValue, ' '); // text node
check_equals(myxml2.toString(), "<X1> </X1>"); 

myxml2.ignoreWhite = true;
myxml2.parseXML("<X1>\n</X1>");
check_equals(myxml2.childNodes.length, 1);
check(!myxml2.childNodes[0].hasChildNodes());
check_equals(myxml2.toString(), "<X1 />"); 

myxml2.ignoreWhite = true; 
myxml2.parseXML("<X1> t </X1>");
check_equals(myxml2.toString(), "<X1> t </X1>"); 

stop();

