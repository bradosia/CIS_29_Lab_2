//============================================================================
// Name        : Lab2
// Author      : Branden Lee
// Date        : 5/02/2018
// Description : Decoding Code3of9 Symbology in XML files
//============================================================================

#include <string>
#include <fstream>
#include <iostream>     // std::cout
#include <vector>       // std::vector
#include <bitset>
#include <list>
#include <regex>
#include <stack>
using namespace std;

/**
 @class FileHandler
 simply reads or writes the decoded or encoded message to a file\n
 */
class FileHandler {
public:
	FileHandler() {
	}
	~FileHandler() {
	}
	/** takes a file stream by reference and opens a file.\n
	 * the reason we do not return the string of the entire ASCII file
	 * is because we want to stream and not waste memory
	 @pre None
	 @post None
	 @param string File name to open
	 @return True on file open successful and false in not
	 */
	bool readLines(string fileName, ifstream& fileStream);
	bool writeLines(string fileName, ofstream& fileStream);
	bool close(fstream& fileStreamIn, fstream& fileStreamOut);
};

/**
 @class HashTable
 A simple hash table \n
 */
template<class T>
class HashTable {
private:
	vector<pair<int, T>> table;
public:
	HashTable();
	HashTable(int size);
	~HashTable() {
	}
	bool insert(int key, T val);
	T at(int key);
	int hash(int key);
};

/**
 @class XMLNode
 XML document node \n
 */
class XMLNode {
private:
	string name; // tag name inside the angled brackets <name>
	string value; // non-child-node inside node <>value</>
	list<XMLNode*> childNodes;
	XMLNode* parentNode;
public:
	XMLNode();
	XMLNode(string name_, XMLNode* parentNode_);
	~XMLNode() {
	}
	void valueAppend(string str);
	string getValue();
	XMLNode* getParent();
	XMLNode* addChild(string str);
	XMLNode* getChild(unsigned int index);
};

/**
 @class XMLParser
 XML document parser \n
 */
class XMLParser {
private:
	unsigned int fileSize, filePos;
	char bufferInChar[2];
	string streamBuffer;
public:
	XMLParser();
	~XMLParser() {
	}
	bool documentStream(istream& streamIn, XMLNode& xmlDoc);
};

/**
 @class Code39CharTable
 A table to convert Code 39 integers to characters \n
 */
class Code39CharTable {
private:
	vector<unsigned int> charIntToCode39IntTable;
	vector<char> Code39IntToCharTable;
public:
	Code39CharTable();
	~Code39CharTable() {
	}
	void buildCode39IntToCharTable();
	bool Code39IntToChar(unsigned int binaryInt, char& charOut);
};

class Product {
private:
	string name;
	double price;
	unsigned int code39IntItem[5];
public:
	string printString();
};

class ProductTable {
private:
	HashTable<Product> code39ItemToProductTable;
public:
	string printString();
};

class Cart {
private:
	vector<Product> productList;
public:
	string printString();
};

class CartList {
private:
	vector<Cart> cartList;
public:
	string printString();
};

/**
 @class Code39IntItem
 Converts a Code 39 Item to the a product \n
 */
class Code39IntItem {
private:
	Code39CharTable* code39CharTable;
	string code39StringItem;
	unsigned int code39IntItem[5];
public:
	Code39IntItem(string code39StringItem_,
			Code39CharTable* code39CharTablePtr);
	~Code39IntItem() {
	}
	bool toProduct(Product& productObj);
	bool toInt(unsigned int* code39IntItem);
};

/**
 @class Parser
 Miscellaneous utilities for parsing data structures \n
 */
class Parser {
private:
	Code39CharTable code39CharTable;
public:
	Parser();
	~Parser() {
	}
	bool productListXMLNodetoObject(const XMLNode& productListXMLNode,
			ProductTable& cartListObject);
	bool cartListXMLNodetoObject(const XMLNode& cartListXMLNode,
			CartList& cartListObject);
};

/*
 * FileHandler Implementation
 */
bool FileHandler::close(fstream& fileStreamIn, fstream& fileStreamOut) {
	fileStreamIn.close();
	fileStreamOut.close();
	return true;
}

bool FileHandler::readLines(string fileName, ifstream& fileStream) {
	fileStream.open(fileName, ios::binary); // Use it to read from a file named data.txt.
	if (fileStream.is_open()) {
		return true;
	}
	return false;
}

bool FileHandler::writeLines(string fileName, ofstream& fileStream) {
	fileStream.open(fileName); // Use it to read from a file named data.txt.
	if (fileStream.is_open()) {
		return true;
	}
	return false;
}

/*
 * HashTable Implementation
 */
template<class T>
HashTable<T>::HashTable() {
	table.resize(100);
}
template<class T>
HashTable<T>::HashTable(int size) {
	table.resize(size);
}
template<class T>
bool HashTable<T>::insert(int key, T val) {
	int attempts = 3;
	int keyOriginal = key;
	bool flag = false;
	for (; attempts > 0; attempts--) {
		key = hash(key);
		if (!table[key]) {
			table[key].first = keyOriginal;
			table[key].second = val;
			break;
		}
	}
	return flag;
}
template<class T>
T HashTable<T>::at(int key) {
	int attempts = 3;
	int keyOriginal = key;
	T ret;
	for (; attempts > 0; attempts--) {
		key = hash(key);
		if (table[key] && table[key].first == keyOriginal) {
			table[key].first = keyOriginal;
			ret = table[key].second;
			break;
		}
	}
	return ret;
}
template<class T>
int HashTable<T>::hash(int key) {
	return (key * 2654435761) % table.size();
}

/*
 * XMLNode Implementation
 */
XMLNode::XMLNode() {
	name = "";
	value = "";
	parentNode = nullptr;
}
XMLNode::XMLNode(string name_, XMLNode* parentNode_) {
	name = name_;
	value = "";
	parentNode = parentNode_;
}
void XMLNode::valueAppend(string str) {
	value.append(str);
}
string XMLNode::getValue() {
	return value;
}
XMLNode* XMLNode::getParent() {
	return parentNode;
}
XMLNode* XMLNode::addChild(string str) {
	XMLNode* childNode = new XMLNode(str, this);
	childNodes.push_back(childNode);
	return childNode;
}
XMLNode* XMLNode::getChild(unsigned int index) {
	XMLNode* nodeReturn = nullptr;
	try {
		nodeReturn = childNodes[index];
	} catch (...) {
		// nothing
	}
	return nodeReturn;
}

/*
 * XMLParser Implementation
 */
XMLParser::XMLParser() {
	fileSize = filePos = 0;
	bufferInChar[1] = 0;
	streamBuffer = "";
}
bool XMLParser::documentStream(istream& streamIn, XMLNode& xmlDoc) {
	/* <primaryAddress>[\s\S]*?<\/primaryAddress>
	 * 1. create document node. If stack is empty then document node is the parent.
	 * 2. grab first <tag> and add push on stack.
	 *    future nodes will be a child of top of the stack
	 * 3. grab child node and push it to the stack.
	 * 4. value between <child></child> is added to the node on top of the stack
	 * 5. if </tag> found then it is popped off the stack
	 * */
	stack<string> documentStack;
	string tagPop;
	XMLNode* xmlNodeCurrent = &xmlDoc;
	streamIn.seekg(0, ios::end); // set the pointer to the end
	fileSize = streamIn.tellg(); // get the length of the file
	streamIn.seekg(0, ios::beg); // set the pointer to the beginning
	std::regex tagOpen("<[\s\S]*?>");   // matches an opening tag <tag>
	std::regex tagEnd("<\/[\s\S]*?>");   // matches an ending tag </tag>
	for (filePos = 0; filePos < fileSize; filePos++) {
		// read one byte at a time
		streamIn.seekg(filePos, ios::beg);
		streamIn.read(bufferInChar, 1);
		streamBuffer.append(bufferInChar);
		if (bufferInChar[0] == '<') {
			/* opening angle bracket for a tag
			 * we assume that text before this is the value of current xml node
			 */
			xmlNodeCurrent->valueAppend(
					streamBuffer.substr(0, streamBuffer.size() - 1));
			streamBuffer = "<";
		} else if (bufferInChar[0] == '>') {
			/* ending angle bracket for a tag
			 * let's use regex to grab the tag name between the angled brackets
			 * let's first check if we just ended an ending tag </>
			 */
			std::smatch m;
			regex_search(streamBuffer, m, tagEnd);
			if (m.size() > 0) {
				tagPop = "";
				while (!documentStack.empty() && tagPop != m[0]) {
					tagPop = documentStack.pop();
					xmlNodeCurrent = xmlNodeCurrent->getParent();
					if (xmlNodeCurrent == nullptr) {
						xmlNodeCurrent = &xmlDoc;
					}
				}
			} else {
				/* now check if we just ended an opening tag <>
				 */
				std::smatch m;
				regex_search(streamBuffer, m, tagOpen);
				if (m.size() > 0) {
					documentStack.push(m[0]);
					xmlNodeCurrent = xmlNodeCurrent->addChild(m[0]);
					if (xmlNodeCurrent == nullptr) {
						xmlNodeCurrent = &xmlDoc;
					}
				}
			}
			streamBuffer = "";
		}
	}
	// remaining buffer belongs to current node value
	xmlNodeCurrent->valueAppend(streamBuffer);
	return true;
}

/*
 * Code39CharTable Implementation
 */
Code39CharTable::Code39CharTable() {
	// Size of 128 to potentially hold ascii codes up to 128
	charIntToCode39IntTable.resize(128);
	try {
		charIntToCode39IntTable[(int) ''] = 196;
		charIntToCode39IntTable[(int) '-'] = 133;
		charIntToCode39IntTable[(int) '+'] = 138;
		charIntToCode39IntTable[(int) '$'] = 168;
		charIntToCode39IntTable[(int) '%'] = 42;
		charIntToCode39IntTable[(int) '*'] = 148;
		charIntToCode39IntTable[(int) '.'] = 388;
		charIntToCode39IntTable[(int) '/'] = 162;
		charIntToCode39IntTable[(int) '0'] = 52;
		charIntToCode39IntTable[(int) '1'] = 289;
		charIntToCode39IntTable[(int) '2'] = 97;
		charIntToCode39IntTable[(int) '3'] = 352;
		charIntToCode39IntTable[(int) '4'] = 49;
		charIntToCode39IntTable[(int) '5'] = 304;
		charIntToCode39IntTable[(int) '6'] = 112;
		charIntToCode39IntTable[(int) '7'] = 37;
		charIntToCode39IntTable[(int) '8'] = 292;
		charIntToCode39IntTable[(int) '9'] = 100;
		charIntToCode39IntTable[(int) 'A'] = 265;
		charIntToCode39IntTable[(int) 'B'] = 73;
		charIntToCode39IntTable[(int) 'C'] = 328;
		charIntToCode39IntTable[(int) 'D'] = 25;
		charIntToCode39IntTable[(int) 'E'] = 280;
		charIntToCode39IntTable[(int) 'F'] = 88;
		charIntToCode39IntTable[(int) 'G'] = 13;
		charIntToCode39IntTable[(int) 'H'] = 268;
		charIntToCode39IntTable[(int) 'I'] = 76;
		charIntToCode39IntTable[(int) 'J'] = 28;
		charIntToCode39IntTable[(int) 'K'] = 259;
		charIntToCode39IntTable[(int) 'L'] = 67;
		charIntToCode39IntTable[(int) 'M'] = 322;
		charIntToCode39IntTable[(int) 'N'] = 19;
		charIntToCode39IntTable[(int) 'O'] = 274;
		charIntToCode39IntTable[(int) 'P'] = 82;
		charIntToCode39IntTable[(int) 'Q'] = 7;
		charIntToCode39IntTable[(int) 'R'] = 262;
		charIntToCode39IntTable[(int) 'S'] = 70;
		charIntToCode39IntTable[(int) 'T'] = 22;
		charIntToCode39IntTable[(int) 'U'] = 385;
		charIntToCode39IntTable[(int) 'V'] = 193;
		charIntToCode39IntTable[(int) 'W'] = 448;
		charIntToCode39IntTable[(int) 'X'] = 145;
		charIntToCode39IntTable[(int) 'Y'] = 400;
		charIntToCode39IntTable[(int) 'Z'] = 208;
	} catch (...) {
		// nothing
	}
	buildCode39IntToCharTable();
}

void Code39CharTable::buildCode39IntToCharTable() {
	unsigned int i, n, n1;
	/* 2^9 since the longest Code 39 Binary is 9 bits */
	Code39IntToCharTable.resize(512);
	// build a binary int to char map
	n = charIntToCode39IntTable.size();
	for (i = 0; i < n; i++) {
		if (charIntToCode39IntTable[i]
				&& (n1 = charIntToCode39IntTable[i]) > 0) {
			// we are worried of bits above 9
			try {
				Code39IntToCharTable[n1] = char(i);
			} catch (...) {
				// nothing
			}
		}
	}
}

bool Code39CharTable::Code39IntToChar(unsigned int binaryInt, char& charOut) {
	//cout << "binaryInt=" << binaryInt << endl;
	try {
		charOut = Code39IntToCharTable[binaryInt];
		return true;
	} catch (...) {
		return false;
	}
}
/*
 * Code39IntItem Implementation
 */
Code39IntItem::Code39IntItem(string code39StringItem_,
		Code39CharTable* code39CharTablePtr) {
	code39StringItem = code39StringItem_;
	code39CharTable = code39CharTablePtr;
	unsigned int i, n;
	for (i = 0; i < 5; i++) {
		bitset<9> bits(code39StringItem.substr(i * 9, 9));
		code39IntItem[i] = (unsigned int) bits.to_ulong();
	}
}

bool Code39IntItem::toProduct(Product* productObj) {
	string code39StringItem;
	toString(code39StringItem);
	productObj = productTable.at(code39StringItem);
	return true;
}
bool Code39IntItem::toInt(unsigned int* code39IntItem) {
	unsigned int i, n;
	for (i = 0; i < 5; i++) {
		bitset<9> bits(code39StringItem.substr(i * 9, 9));
		code39IntItem[i] = (unsigned int) bits.to_ulong();
	}
	return true;
}
bool Code39IntItem::toString(string& code39StringItem) {
	unsigned int i, n;
	unsigned int code39IntItem[5];
	toInt(code39IntItem);
	for (i = 0; i < 5; i++) {
		code39StringItem[i] = code39CharTable.at(code39IntItem[i]);
	}
	return true;
}

/*
 * Parser Implementation
 */
Parser::Parser() {

}

/*
 * main & interface
 * Rules For Decoding:
 * - it is assumed all morse binary strings end with a letter space or word space
 * - only morse binary strings matching the pre-built table can be decoded
 * - morse binary strings a length 0 <= x <= 14 will be ignored
 * - after ignoring 3 bad morse binary strings the parsing will stop
 */
int main() {
	FileHandler fh;
	Parser parser;
	XMLParser xmlparser;
	string fileNameProducts, fileNameCarts;
	ifstream fileStreamInProducts, fileStreamInCarts;
	fileNameProducts = "Products.xml";
	fileNameCarts = "Carts.xml";
	// parse XML file streams into an XML document node
	XMLNode ProductsXML, CartsXML;

	if (!fh.readLines(fileNameProducts, fileStreamInProducts)
			|| !fh.readLines(fileNameCarts, fileStreamInCarts)) {
		cout << "Could not read either of the input files." << endl;
	} else {
		cout << "Reading file..." << endl;
		/* we pass file streams instead of a string to this method
		 * because we want to stream the data and decode it as we read.
		 * This way very large files won't lag or crash the program.
		 */
		if (xmlparser.documentStream((istream&) fileStreamInProducts,
				ProductsXML)
				&& xmlparser.documentStream((istream&) fileStreamInCarts,
						CartsXML)) {
			cout << "XML Files successfully parsed!" << endl;

		} else {
			cout << "XML Files could not be parsed." << endl;
		}
		fh.close((fstream&) fileStreamInProducts, (fstream&) fileStreamInCarts);
	}
	cout << "Enter any key to exit..." << endl;
	string temp;
	getline(cin, temp);
	return 0;
}
