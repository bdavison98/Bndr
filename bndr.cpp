#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <iterator>
#include <algorithm>
#include <random>


/* This struct is used to store a char vector to hold bytes of the file, 
   a int vector to hold byte locations within the file buffer, 
   and two boolean variables used to show if the respective vectors are null or not  */
struct Buffer {
	std::vector<int> intbuffer;
	bool intIsNull = true;
	std::vector<char> charbuffer;
	bool charIsNull = true;
};


/* The 54th byte in a bmp file is basically the location of the first pixel 
   in the image, and all the following bytes are also pixels until EOF */
int BMP_PIXELS_START = 54;


// small function to check if the file exists by opening it
bool doesFileExist(std::string const& filename) {
	std::ifstream file(filename);
	return file.is_open();
}


// fucntion to check if the file is valid 
bool isFileValid(std::string filename) {

	// checks if the file does not exist
	if (!doesFileExist(filename)) {

		std::cout << filename << " does not exist! Please enter a valid file\n";

		// continues to get user input for a filename until the file truly exists
		while (std::cin >> filename) {
			if (doesFileExist(filename)) {
				break;
			}
			else {
				std::cout << filename << " does not exist! Please enter a valid file\n";
				continue;
			}
		}
	}
	// checks if the file is not a bmp file
	else if (!filename.find(".bmp")) {

		std::cout << "Only bitmap images are supported.\nPlease enter a path to a bmp image\n";

		// continues to get user input for a filename until the file is truly a bitmap image
		while (std::cin >> filename) {
			if (filename.find(".bmp")) {
				break;
			}
			else {
				std::cout << "Only bitmap images are supported.\nPlease enter a path to a bmp image\n";
				continue;
			}
		}
	}
	return true;
}


// function to handle yes/no user input
bool getYesNo() {

	std::cout << "\nY/N?: ";
	char response;

	while (true) {

		std::cin >> response;

		switch (response) {
		case 'y':
			return true;
			break;
		case 'Y':
			return true;
			break;
		case 'n':
			return false;
			break;
		case 'N':
			return false;
			break;
		default:
			std::cout << "\nEnter 'y' for yes, or 'n' for no.\n";
			continue;
		}
	}
}


// function to save the modified buffer as a file
void save(std::vector<char> buffer, std::string filename) {

	std::cout << "\nSave as " << filename << "?\n";
	
	if (!getYesNo()) {

		std::string newName;
		std::cout << "\nEnter a new filename...\n";
		std::cin >> newName;
	
		filename = newName;
	}
	std::ofstream newFile(filename);

	// loop to save the modified bytes into a new file
	for (std::vector<char>::const_iterator iterator = buffer.begin(); iterator != buffer.end(); ++iterator) {
		newFile << *iterator;
	}

	// makes sure that the file was saved
	if (doesFileExist(filename)) {
		std::cout << "\n" << filename << " saved \n";
	}

}


// function to search a given buffer for a target
Buffer searchBuffer(std::vector<char> buffer, const char target) {

	std::vector<char>::iterator iterator,temp;

	std::vector<char> targetContainer = {target};

	std::vector<int> positions;
	
	// searches the buffer from start to end for the target Vector
	iterator = std::search(buffer.begin(), buffer.end(), targetContainer.begin(), targetContainer.end());

	int currPos = 0;

	// adds the location of the target to the posistions vector until the end of the iterator is reached
	while (iterator != buffer.end()) {

		currPos = int(iterator - buffer.begin()) ;
		
		positions.push_back(currPos);
		
		// searches the buffer from the last position plus the size of the target vector so the next loop will look for the next occurence of the target
		iterator = std::search(buffer.begin() + currPos + targetContainer.size(), buffer.end(), targetContainer.begin(), targetContainer.end());
	
	}
	if (positions.size() == 0) {
		std::cout << target << std::hex << "NOT FOUND\n";
		
		return { {}, true, buffer, false };
	}
	else {
		return { positions,false, {}, true };
	}
}


/* function to modify buffer by replacing char p with char r for every occurence found
 enableLimit allows the user to limit the number of replacements done for each char p to 1500. */
Buffer replace(std::vector<char> buffer, const char p, const char r, bool enableLimit) {
	
	// Holds search return vector
	std::vector<int> positions, imagePos;
	std::vector<char> pattern = { p };
	std::vector<char> replacement = { r };



	int pos= 0,j=0,repTotal = 0, pixelLoc = BMP_PIXELS_START;
	
	char repl;

	if(pattern.size() != 0 || replacement.size() != 0 || buffer.size() != 0 ){

		// finds all occurences of char p and stores each position in a vector
		positions = searchBuffer(buffer, pattern.at(0)).intbuffer;
		
		// if the size of positions is 0, then no matches were found for char p
		if (positions.size() == 0) {
			return { { }, true, buffer, false};
		}

		// loop that replaces char p with char repl at position pos from the position vector
		for (int i = 0; i < positions.size() && pos < buffer.size(); i++) {
			
			// the replacement total reaches 1500, stop the loop
			if (enableLimit) {
				if (repTotal == 1500) break;
			}
			// gets nth location of char p 
			pos = positions.at(i);
			
			// replacement needs to start on or past byte 54
			pos += pixelLoc;
			
			repl = replacement.at(0);
			
			int replacementLoc = __int64(pos);
			// 
			if (replacementLoc < buffer.size()) {
				buffer.at(replacementLoc) = repl;
			}
			repTotal++;

		}
		std::cout << "\nReplaced " << std::hex << pattern.at(0)  << " with " << std::hex << replacement.at(0)<< " " << std::dec << repTotal << " times.\n";
		
		return { {},true,buffer,false };
	}
	else {
		
		std::cout << "Check params";
		
		return {{}, true, buffer, false};
	}
}


// function that find and replace specific bytes
Buffer patternReplace(std::vector<char> buffer,std::vector<char> find, std::vector<char> replacement, bool enableLimit) {

	if (find.size() == 0 || replacement.size() == 0) {

		std::cout << "Pattens cannot be empty!";

		return { {},true, buffer,false };
	}
	else {
		for (int i = 0;i<find.size() && i<replacement.size();i++) {
			buffer = replace(buffer, find.at(i),replacement.at(i), enableLimit).charbuffer;
		}
		return { {},true,buffer,false };
	}
}


// function that randomly generates bytes to find and replace
Buffer randomReplace(std::vector<char> buffer, int patternLen, bool enableLimit) {

	// subtract 1 because the loop below starts at 0
	patternLen -= 1;
	
	std::vector<char> findVec(patternLen), replaceVec(patternLen);

	int recomendedBlockSize = buffer.size() / 0.125;

	if (patternLen == -1) {
		std::cout << "N cannot be 0.\nNo changes were made to the file.";
		return { {}, true, buffer, false };
	}
	// fill vectors with random bytes up to the given pattern length
	for(int i = 0; i <= patternLen; i++) {

		// generating random bytes
		int randF = std::rand() % 255;
		int randR = std::rand() % 255;

		findVec.push_back(randF);
		replaceVec.push_back(randR);
		
	}
	
	std::vector<char>::iterator i = findVec.begin(), j = replaceVec.begin();

	// loop to call replace function using each element within the find and replace vectors as an arguement
	for (; i != findVec.end() && j != replaceVec.end(); ++i,++j) {
		buffer = replace(buffer, (*i), (*j), enableLimit).charbuffer;
	}

	return { {} ,true,buffer,false };
	
}


Buffer getHeader(Buffer buf, std::string filename) {
	if (filename.find(".jpg")) {
		
	}
}


int main() {

	std::cout << "Bndr: Databending Program\n\n\n";

	char response;

	std::ifstream::pos_type size = 0;
	
	std::ifstream file;
	std::string filename;
	
	
	std::cout << "Enter the path to a image\n";
	std::cin >> filename;

	if (isFileValid(filename)){
		file.open(filename, std::ios::binary | std::ios::in | std::ios::ate);
	}

	// sanity check
	if (file.is_open()) {
		
		// getting the file size
		size = file.tellg();
		file.seekg(0, std::ios::beg);
		
		// creating a buffer structure

		Buffer buf;
		buf.charbuffer = std::vector<char>(size);
		
		// reads the bytes of the file into the buffer vector
		if (file.read(buf.charbuffer.data(), size)) {

			buf.charIsNull = false;
			bool limit = false;

			std::vector<char> find, repl;
			
			std::cout << "There are two modes of glitching: Automatic (0) or Manual (1)\nEnter the mode of your choice using 0 or 1...\nMode: ";

			while(true) {

				// getting user input for the replacement mode
				int mode;
				std::cin >> mode;

				int vectorSize=0,i=0,j=0,patternLen = 0;
				
				switch(mode) {
					case 0:
						std::cout << "\nLimit the number of bytes replaced?";

						if (getYesNo()) {
							// allows the user to limit the number of replacements to 1500 per unique byte
							limit = true;
						}
						
						std::cout << "\nEnter a number to be used as the length of a random pattern.\n[Warning: High pattern lengths will take longer to replace]\n";
						std::cin >> patternLen;

						buf.charbuffer = randomReplace(buf.charbuffer, patternLen, limit).charbuffer;

						break;
					case 1:
						char input;
						
						std::cout << "\nEnter the number of unique bytes you want to find.\n";
						std::cin >> vectorSize;

						std::cout << "\nEnter the pattern you want to find byte by byte in the form 0xFF\n";

						// filling the vector that holds the pattern
						while ((std::cin >> input) && i < vectorSize) {
							find.push_back(input);
							i++;
						}

						std::cout << "\nEnter the replacement pattern byte by byte in the form 0xFF\n";

						// filling the vector that replaces the pattern
						while ((std::cin >> input) && j < vectorSize) {
							repl.push_back(input);
							j++;
						}

						buf.charbuffer = patternReplace(buf.charbuffer, find, repl, false).charbuffer;

						break;
					default:

						std::cout << "0 and 1 are the only two modes!\n";
						std::cout << "Please enter a valid mode...\nMode: ";

						continue;
					}
				
				// just to be safe
				if (!buf.charIsNull) {
					save(buf.charbuffer, filename);
				}
				else {
					std::cout << "Error saving file! Would you like to retry?";
					if (getYesNo()) {
						continue;
					}
					else { 
						break;
					}
				}
				break;


			}
		
 		}
	}
	file.close();

	return 0;
}


/*				TO DO
	if (filename.find(".mp4")) {
		1. start replacement after important parts of the header
								OR
		1. fix header of file before saving the file
		2. more specific tweaks for each file format
		3. for .mp4, limit the range of random to [0,9].
	}
*/