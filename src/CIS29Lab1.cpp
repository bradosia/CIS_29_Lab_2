//============================================================================
// Name        : Lab1
// Author      : Branden Lee
// Date        : 4/22/2018
// Description : Decode morse binary code
//============================================================================

#include <string>
#include <fstream>
#include <iostream>     // std::cout
#include <vector>       // std::vector
#include <bitset>
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
	bool close(ifstream& fileStreamIn, ofstream& fileStreamOut);
};

/**
 @class MorseBinaryCharTable
 It converts hexadecimals to chars \n
 */
class MorseBinaryCharTable {
private:
	vector<string> charIntToBinaryStringTable;
	vector<char> binaryIntToCharTable;
	const string dashBinary = "01";
	const string dotBinary = "10";
public:
	MorseBinaryCharTable();
	~MorseBinaryCharTable() {
	}
	void buildBinaryIntToCharTable();
	bool binaryIntToChar(unsigned int binaryInt, char& charOut);
};

/**
 @class MorseBinary
 Utilities for pairs of degree angles \n
 */
class MorseBinary {
private:
	unsigned int binaryInt;
	MorseBinaryCharTable binaryCharTable;
public:
	MorseBinary(string morse);
	~MorseBinary() {
	}
	bool toChar(char& charOut);
	bool bitStringToInt(string morse, unsigned int& intOut);
};

/**
 @class Parser
 Utilities for stream decoding \n
 */
class Parser {
private:
	unsigned int fileSize, filePos, charWritten, ignoreNum;
	char bufferInChar[2], bufferOutChar[2], spaceChar[2];
	string streamBuffer;
	bool spaceWrite;
public:
	Parser();
	~Parser() {
	}
	bool morseBinaryToChar(ifstream& fileStreamIn, ofstream& fileStreamOut);
	bool bufferHandle();
	string byteToBitString(char byteIn);
	int bitStringFind(string space);
};

/*
 * MorseBinaryCharTable Implementation
 */
MorseBinaryCharTable::MorseBinaryCharTable() {
	// Size of 128 to potentially hold ascii codes up to 128
	charIntToBinaryStringTable.resize(128);
	try {
		charIntToBinaryStringTable[(int) 'A'] = ".-";
		charIntToBinaryStringTable[(int) 'B'] = "-...";
		charIntToBinaryStringTable[(int) 'C'] = "-.-.";
		charIntToBinaryStringTable[(int) 'D'] = "-..";
		charIntToBinaryStringTable[(int) 'E'] = ".";
		charIntToBinaryStringTable[(int) 'F'] = "..-.";
		charIntToBinaryStringTable[(int) 'G'] = "--.";
		charIntToBinaryStringTable[(int) 'H'] = "....";
		charIntToBinaryStringTable[(int) 'I'] = "..";
		charIntToBinaryStringTable[(int) 'J'] = ".---";
		charIntToBinaryStringTable[(int) 'K'] = "-.-";
		charIntToBinaryStringTable[(int) 'L'] = ".-..";
		charIntToBinaryStringTable[(int) 'M'] = "--";
		charIntToBinaryStringTable[(int) 'N'] = "-.";
		charIntToBinaryStringTable[(int) 'O'] = "---";
		charIntToBinaryStringTable[(int) 'P'] = ".--.";
		charIntToBinaryStringTable[(int) 'Q'] = "--.-";
		charIntToBinaryStringTable[(int) 'R'] = ".-.";
		charIntToBinaryStringTable[(int) 'S'] = "...";
		charIntToBinaryStringTable[(int) 'T'] = "-";
		charIntToBinaryStringTable[(int) 'U'] = "..-";
		charIntToBinaryStringTable[(int) 'V'] = "...-";
		charIntToBinaryStringTable[(int) 'W'] = ".--";
		charIntToBinaryStringTable[(int) 'X'] = "-..-";
		charIntToBinaryStringTable[(int) 'Y'] = "-.--";
		charIntToBinaryStringTable[(int) 'Z'] = "--..";
		charIntToBinaryStringTable[(int) '0'] = "-----";
		charIntToBinaryStringTable[(int) '1'] = ".----";
		charIntToBinaryStringTable[(int) '2'] = "..---";
		charIntToBinaryStringTable[(int) '3'] = "...--";
		charIntToBinaryStringTable[(int) '4'] = "....-";
		charIntToBinaryStringTable[(int) '5'] = ".....";
		charIntToBinaryStringTable[(int) '6'] = "-....";
		charIntToBinaryStringTable[(int) '7'] = "--...";
		charIntToBinaryStringTable[(int) '8'] = "---..";
		charIntToBinaryStringTable[(int) '9'] = "----.";
		charIntToBinaryStringTable[(int) '\''] = ".----.";
		charIntToBinaryStringTable[(int) '@'] = ".--.-.";
		charIntToBinaryStringTable[(int) ':'] = "---...";
		charIntToBinaryStringTable[(int) ','] = "--..--";
		charIntToBinaryStringTable[(int) '$'] = "...-..-";
		charIntToBinaryStringTable[(int) '='] = "-...-";
		charIntToBinaryStringTable[(int) '!'] = "-.-.--";
		charIntToBinaryStringTable[(int) '.'] = ".-.-.-";
		charIntToBinaryStringTable[(int) '?'] = "..--..";
		charIntToBinaryStringTable[(int) '\"'] = ".-..-.";
	} catch (...) {
		// nothing
	}
	buildBinaryIntToCharTable();
}

void MorseBinaryCharTable::buildBinaryIntToCharTable() {
	unsigned int i, j, n, n1;
	string bitString;
	/* 2^14 since $ has the longest binary representation at 14 bits
	 * There is a lot of wasted spaces since we don't even have 2^14
	 * values to store, but we want faster lookup performance compared to smaller memory usage.
	 * Hashing would use less space, at the expense of a few CPU calculations */
	binaryIntToCharTable.resize(16384);
	// build a binary int to char map
	n = charIntToBinaryStringTable.size();
	for (i = 0; i < n; i++) {
		bitString = "";
		if ((n1 = charIntToBinaryStringTable[i].length()) > 0) {
			for (j = 0; j < n1; j++) {
				if (charIntToBinaryStringTable[i][j] == '-') {
					bitString.append(dashBinary);
				} else if (charIntToBinaryStringTable[i][j] == '.') {
					bitString.append(dotBinary);
				}
			}
			bitset<14> bits(bitString);
			//cout << bits.to_ulong() << " " << char(i) << endl;
			// we are worried of bits above 14
			try {
				binaryIntToCharTable[bits.to_ulong()] = char(i);
			} catch (...) {
				// nothing
			}
		}
	}
}

bool MorseBinaryCharTable::binaryIntToChar(unsigned int binaryInt,
		char& charOut) {
	//cout << "binaryInt=" << binaryInt << endl;
	try {
		charOut = binaryIntToCharTable[binaryInt];
		return true;
	} catch (...) {
		return false;
	}
}
/*
 * MorseBinary Implementation
 */
MorseBinary::MorseBinary(string morse) {
	bitStringToInt(morse, binaryInt);
}

bool MorseBinary::toChar(char& charOut) {
	return binaryCharTable.binaryIntToChar(binaryInt, charOut);
}

bool MorseBinary::bitStringToInt(string morse, unsigned int& intOut) {
	bitset<14> bits(morse);
	intOut = (unsigned int) bits.to_ulong();
	return true;
}

/*
 * FileHandler Implementation
 */
bool FileHandler::close(ifstream& fileStreamIn, ofstream& fileStreamOut) {
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
 * Parser Implementation
 */
Parser::Parser() {
	fileSize = filePos = charWritten = ignoreNum = 0;
	bufferInChar[1] = bufferOutChar[1] = spaceChar[1] = 0;
	streamBuffer = "";
	spaceChar[0] = ' ';
	spaceWrite = false;
}
bool Parser::morseBinaryToChar(ifstream& fileStreamIn,
		ofstream& fileStreamOut) {
	bool flag = false;
	fileStreamIn.seekg(0, ios::end); // set the pointer to the end
	fileSize = fileStreamIn.tellg(); // get the length of the file
	fileStreamIn.seekg(0, ios::beg); // set the pointer to the beginning
	for (filePos = 0; filePos < fileSize; filePos++) {
		// read one byte at a time
		fileStreamIn.seekg(filePos, ios::beg);
		fileStreamIn.read(bufferInChar, 1);
		streamBuffer.append(byteToBitString(bufferInChar[0]));
		// keep parsing as long as letter and word spaces are found
		while (bufferHandle()) {
			// successfully found and parsed a morse binary
			fileStreamOut.write(bufferOutChar, 1);
			// check if a space needs to be written after buffer parsing
			if (spaceWrite) {
				spaceWrite = false;
				fileStreamOut.write(spaceChar, 1);
			}
			charWritten += 2;
		}
		// check if a space needs to be written after no buffer parsing
		if (spaceWrite) {
			fileStreamOut.write(spaceChar, 1);
		}
		// to many bad angles, stop parsing
		if (ignoreNum >= 3) {
			flag = false;
		}
	}
	if (charWritten > 0) {
		flag = true;
	}
	return flag;
}

bool Parser::bufferHandle() {
	bool flag = false;
	int letterSpaceNext = bitStringFind("00");
	int wordSpaceNext = bitStringFind("11");
	//cout << letterSpaceNext << " " << wordSpaceNext << endl;
	unsigned int morseEndPos;
	// find letter and word spaces
	if (letterSpaceNext != -1 || wordSpaceNext != -1) {
		if ((letterSpaceNext != -1 && wordSpaceNext == -1)
				|| (letterSpaceNext != -1 && wordSpaceNext != -1
						&& letterSpaceNext < wordSpaceNext)) {
			morseEndPos = (unsigned int) letterSpaceNext;
			spaceWrite = false;
		} else {
			morseEndPos = (unsigned int) wordSpaceNext;
			spaceWrite = true;
		}
		//cout << "streamBuffer=" << streamBuffer << endl;
		//cout << "morseEndPos=" << morseEndPos << endl;
		string morseBitString = streamBuffer.substr(0, morseEndPos);
		streamBuffer.erase(0, morseEndPos + 2);
		// morse bits extracted. maximum morse bit string is 14
		if (morseBitString.length() > 0 && morseBitString.length() <= 14) {
			//cout << "morseBitString=" << morseBitString << endl;
			MorseBinary morseBinary(morseBitString);
			if (morseBinary.toChar(bufferOutChar[0])) {
				flag = true;
			} else {
				// conversion not good. let's ignore this char
				ignoreNum++;
			}
		} else {
			ignoreNum++;
		}
	}
	return flag;
}

string Parser::byteToBitString(char byteIn) {
	string bitString = "";
	for (int i = 7; i >= 0; i--) {
		bitString.append(to_string((byteIn >> i) & 1));
	}
	return bitString;
}

int Parser::bitStringFind(string space) {
	string bufferBits2 = "";
	int pos = -1;
	unsigned int i, n;
	n = streamBuffer.size();
	for (i = 0; i < n; i++) {
		if (i % 2 == 1) {
			if (streamBuffer.substr(i - 1, 2) == space) {
				return (int) (i - 1);
			}
		}
	}
	return pos;
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
	string fileNameIn, fileNameOut;
	ifstream fileStreamIn;
	ofstream fileStreamOut;
	/*cout << "Please enter the input file name and extension:" << endl;
	 getline(cin, fileNameIn);
	 cout << "Please enter the output file name and extension:" << endl;
	 getline(cin, fileNameOut);*/
	fileNameIn = "Morse.bin";
	fileNameOut = "output.txt";
	if (!fh.readLines(fileNameIn, fileStreamIn)
			|| !fh.writeLines(fileNameOut, fileStreamOut)) {
		cout << "Could not read either the input or output file." << endl;
	} else {
		cout << "Reading file..." << endl;
		/* we pass file streams instead of a string to this method
		 * because we want to stream the data and decode it as we read.
		 * This way very large files won't lag or crash the program.
		 */
		if (parser.morseBinaryToChar(fileStreamIn, fileStreamOut)) {
			cout << "File successfully decoded!" << endl;
		} else {
			cout << "File could not be decoded." << endl;
		}
		fh.close(fileStreamIn, fileStreamOut);
	}
	cout << "Enter any key to exit..." << endl;
	string temp;
	getline(cin, temp);
	return 0;
}
