//#include "PokerLibrary/omp/Hand.h"
//#include "PokerLibrary/omp/OffsetTable.hxx"
//#include "PokerLibrary/omp/Util.h"
//#include "PokerLibrary/omp/Random.h"
//#include "PokerLibrary/omp/EquityCalculator.h"
//#include "PokerLibrary/omp/Constants.h"
//#include "PokerLibrary/omp/CombinedRange.h"
//#include "PokerLibrary/omp/CardRange.h"
//#include "PokerLibrary/libdivide/libdivide.h"
#pragma once
#include "stdafx.h"
#include "../../PokerLibrary/PokerLibrary/omp/HandEvaluator.h" //PokerLibrary/HandEvaluator.h"
#include <iostream>
#include<random>
#include <fstream>
#include <string>
#include <vector>
#include <numeric> 
#include <algorithm>
#include <iterator>
#include <sys/types.h>
#include <sys/stat.h>
#include "time.h"
#include <chrono>
#include <utility>
#include <cstring>
#include <experimental/filesystem>
#include <filesystem>
#include <regex>
#include <queue>
#include <exception>
#include<windows.h>
#include<stdio.h>
#include<WCHAR.h>
#include <thread>
#include <future>
#include <functional>

//writing data to a file (overwrites current data)
void writeToFile(std::string name, std::string text)
{
	
	std::ofstream myfile;
	myfile.open(name);
	//succesfully opened the file
	if (myfile.is_open()) {
		myfile << text;
		myfile.close();
		
	}
	//not succesful- try again
	else {
		myfile.close();
		std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10 + 1));
		writeToFile(name, text);
	}
}

//writing data to read-only file (overwrites current data), wait and retry when encountering an error
void writeToReadOnlyFile(WCHAR userPath[], char dataBuffer[]) {
	//remove the read only tag for next step
	SetFileAttributes(userPath, FILE_ATTRIBUTE_NORMAL);
	//open another handle, which locks the file from other processes while writing 
	HANDLE hFile=NULL;
	//writing to a read only file using win32 API CreateFile() - thus this works on Windows only (possible to have multiple system specific implementations)
	hFile = CreateFile(userPath,                // name of the write
		GENERIC_WRITE,          // open for writing
		0,                      // do not share
		NULL,                   // default security
		OPEN_EXISTING,          // open existing file
		FILE_ATTRIBUTE_NORMAL,  // normal file
		NULL);
	BOOL bErrorFlag = FALSE;
	DWORD dwBytesToWrite = (DWORD)strlen(dataBuffer);
	DWORD dwBytesWritten = 0;

	//attempt writing to the file
	bErrorFlag = WriteFile(
		hFile,           // open file handle
		dataBuffer,      // start of data to write
		dwBytesToWrite,  // number of bytes to write
		&dwBytesWritten, // number of bytes that were written
		NULL);

	//if the data cannot be written, bErrorFlag is false
	if (bErrorFlag == FALSE)
	{
		DWORD lastError = GetLastError();
		//int error;
		//error << lastError;
		std::cout << "error " << lastError << " in writeToReadOnlyFile, unable to write to a file \n";
		//retry after a wait period of between 1-10 ms
		std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10 + 1));
		writeToReadOnlyFile(userPath, dataBuffer);
	}
	//succesfully able to open the handle
	else
	{
		//did not write all the data, some error encountered
		if (dwBytesWritten != dwBytesToWrite)
		{
			printf("Error: dwBytesWritten != dwBytesToWrite\n");
		}
		//successful write with all data appended 
		else
		{
			//std::cout << "Wrote " << dwBytesWritten << " bytes to file successfully.\n";
		}
	}
	//set the file back to read-only
	SetFileAttributes(userPath, FILE_ATTRIBUTE_READONLY);
	//close handle
	CloseHandle(hFile);
}

//create a read-only file-typically used once, currently waits and retries after encountering an error
void createReadOnlyFile(WCHAR userPath[]) {
	SetFileAttributes(userPath, FILE_ATTRIBUTE_NORMAL);
	//read only play area file implementation
	//path to the file to be created, including the name of the file(and extension) - create once
	HANDLE hFile = NULL;
	//creating a read only file using win32 API CreateFile() - thus this works on Windows only (possible to have multiple system specific implementations)
	hFile = ::CreateFile(userPath,
		GENERIC_READ | GENERIC_WRITE, // open for reading and writing
		FILE_SHARE_READ, // read share
		0, // default security
		CREATE_ALWAYS, // always create new one, even if it exists (this will overwrite the data currently in file)
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_READONLY,
		0);
	//if the file cannot be created, tries again after a delay of between 1-10ms
	if (CloseHandle(hFile) == 0)
	{
		DWORD lastError = GetLastError();
		//int error;
		//error<< lastError;
		std::cout << "error " << lastError << " in createReadOnlyFile, unable to create a file \n";
		std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10 + 1));
		createReadOnlyFile(userPath);
	
		
	}
	//close the handle
	CloseHandle(hFile);

}

//appending data to end of file
void appendToFile(std::string name, std::string text)
{

	std::ofstream myfile;
	myfile.open(name, std::ios_base::app);
	//succesfully opened the file
	if (myfile.is_open()) {
		myfile << text;
		myfile.close();

	}
	//not succesful- try again
	else {
		myfile.close();
		std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10 + 1));
		appendToFile(name, text);
	}

	

}

//appending data to end of the read-only file- file will not be able to be accessed while this is happening, retries after a delay of 1-10ms
void appendToReadOnlyFile(WCHAR userPath[], char dataBuffer[]) {
	//remove the read only tag for next step
	SetFileAttributes(userPath, FILE_ATTRIBUTE_NORMAL);
	//open another handle, which locks the file from other processes while writing (append mode) - note the FILE_APPEND_DATA attribute
	HANDLE hFile = NULL;
	hFile = CreateFile(userPath,                // name of the write
		FILE_APPEND_DATA,          // open for writing(append)
		0,                      // do not share
		NULL,                   // default security
		OPEN_EXISTING,             // create new file only
		FILE_ATTRIBUTE_NORMAL,  // normal file
		NULL);
	BOOL bErrorFlag = FALSE;
	DWORD dwBytesToWrite = (DWORD)strlen(dataBuffer);
	DWORD dwBytesWritten = 0;

	//attempt writing to the file
	bErrorFlag = WriteFile(
		hFile,           // open file handle
		dataBuffer,      // start of data to write
		dwBytesToWrite,  // number of bytes to write
		&dwBytesWritten, // number of bytes that were written
		NULL);

	//if the data cannot be written, bErrorFlag is false
	if (bErrorFlag == FALSE)
	{
		DWORD lastError = GetLastError();
		//int error;
		//error << lastError;
		std::cout << "error " << lastError << " in appendToReadOnlyFile, unable to append to a file \n";
		std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10 + 1));
		appendToReadOnlyFile(userPath, dataBuffer);
	}
	//succesfully able to open the handle
	else
	{
		//did not write all the data, some error encountered
		if (dwBytesWritten != dwBytesToWrite)
		{
			std::cout<<"Error: dwBytesWritten != dwBytesToWrite\n";
		}
		//successful write with all data appended 
		else
		{
			//std::cout << "Wrote " << dwBytesWritten << " bytes to file successfully.\n";
		}
	}
	//set the file back to read-only
	SetFileAttributes(userPath, FILE_ATTRIBUTE_READONLY);
	//close the handle
	CloseHandle(hFile);
}

//outputs the contents of a file to the standard display- debug tool
std::string readFromFileDebug(std::string name)
{
	std::ifstream input(name);
	std::string line;
	//succesfully opened the file
	if (input.is_open()) {
	while (std::getline(input, line)) {
	std::cout << line << '\n';
	}
	input.close();
		

	}
	//not succesful- try again
	else {
		input.close();
		std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10 + 1));
		line= readFromFileDebug(name);
	}

	return line;

	

}

//reading data from file
std::string readFromFile(std::string name)
{
	std::ifstream input(name);
	std::string line;
	std::string output="";

	//succesfully opened the file
	if (input.is_open()) {
		while (std::getline(input, line)) {
			output += line + '\n';
		}
		input.close();
		

	}
	//not succesful- try again
	else {
		input.close();
		std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10 + 1));
		output = readFromFile(name);
	}

	return output;





}

//reading data from config file
std::vector<std::pair<std::string, std::string>> readFromConfigFile(std::string name)
{
	std::ifstream input(name);
	std::string nameOfConfig;
	std::string value;
	std::vector<std::pair<std::string, std::string>> output;
	char delimeter('-'); //It is always '-'

	//succesfully opened the file
	if (input.is_open()) {
		
		while (std::getline(input, nameOfConfig, delimeter)) {
			std::getline(input, value);
			output.push_back(std::pair<std::string, std::string>(nameOfConfig, value));
		}
		input.close();


	}
	//not succesful- try again
	else {
		std::cout << "Problem, check path and/or read/write permissions. Path given: " << name<<std::endl;
		input.close();
		std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10 + 1));
		output = readFromConfigFile(name);
	}

	return output;

}



//display contents of a vector (mostly used for debug)
template<typename T>
void displayVector(const std::vector<T> &v)
{
	std::copy(v.begin(), v.end(),
		std::ostream_iterator<T>(std::cout, "\n"));
}

// takes a vector, index of element, returns name from the path at index e
std::string nameFromPath(std::vector<std::string> &a, int e)
{	
	//this function is used for files that the extension is 3 characters long, and also removes the dot
	//if you want to use it on extensions with different length, use extensionLength= 1(dot) + extensionlength + 1 (for iteretor)
	static constexpr int extenstionLength = 5;
	std::string stuffs;
	for (auto i = a[e].end() - extenstionLength; i != a[e].begin(); --i) {
		//detects if there are no more files to be read
		if (*i == '\\')
		{
			return stuffs;
		}
		stuffs.insert(stuffs.begin(), *i);

	}
	return "";

}

//fetches the name of the file i.e extract all directory and extension information
std::vector<std::string> fetchTxtFromDirectory(std::string path) 
{
	std::regex rx(".+\\.txt$", std::regex::icase);
	std::string r;
	std::vector<std::string> i;
	for (auto & p : std::experimental::filesystem::v1::directory_iterator(path))
	{
		if (std::regex_match(p.path().string(), rx))
		{
			i.push_back(p.path().string());
			//std::cout << p.path().string() << std::endl;
		}
	}	return i;

}


//make sure the the directory doesn't have any other .txt files, other than the player ones
//currently exits if the amount of querried files is less than the amount of files in directory
//might be inefficient, possibly room for optimisation? (check if more files were added before running?)
std::vector<std::string> fetchTxtFromDirectory(std::string path, int players) 
{
	//regular expression for detecting .txt files
	std::regex rx(".+\\.txt$", std::regex::icase);
	std::string r;
	std::vector<std::string> i;
	for (auto & p : std::experimental::filesystem::v1::directory_iterator(path))
	{
		if (i.size() == players)
		{
			return i;
		}
		if (std::regex_match(p.path().string(), rx))
		{
			i.push_back(p.path().string());
			//std::cout << p.path().string() << std::endl;
		}
	}
	if (i.size() < players)
	{
		std::cout<<"amount of files is less than the amount querried";
		std::_Exit(EXIT_FAILURE);

	}
	else
	{
		return i;
	}
}




//evaliutates players hands, given a queue of players with cards, the boardCards and the cards
std::queue<std::tuple<std::string, int, int, int, char>> evaluatePlayerHands(int boardCards,std::vector<int> cards,std::queue<std::tuple<std::string, int, int, int, char >> playerQueue)
{

	//in here, initilaise the river + turn + flop, and then get hand values of each individual player
	//this will most likely be done in a seperate thread, while waiting for playerbots to make their decisions
	using namespace omp;
	HandEvaluator eval;
	Hand h = Hand::empty(); // Final hand must include empty() exactly once!

	//creating the hand object for the board cards
	for (int i = 0; i < boardCards; i++)
	{
		h += Hand(cards[i]);
	}
	std::queue<std::tuple<std::string, int, int, int, char>> currentPlayers(playerQueue);
	//creating hand object for each player in the queue
	for (int i = 0; i < currentPlayers.size(); i++)
	{
		//Number stored represents a cardId, which is is an integer between 0 and 51, such that that CARD = 4 * RANK + SUIT, where
		// rank ranges from 0 (deuce) to 12 (ace) and suit is from 0 (spade) to 3 (diamond).
		Hand PlayerHand = h;
		PlayerHand += Hand(std::get<1>(currentPlayers.front())) + Hand(std::get<2>(currentPlayers.front()));
		//store the score of the player in the first slot
		std::get<1>(currentPlayers.front()) = eval.evaluate(PlayerHand);
		currentPlayers.push(currentPlayers.front());
		currentPlayers.pop();
	}
	return currentPlayers;
}



//declares the winner of the hand, based on their scores, as calculated by the evalutaor
std::vector<std::string> declareWinner(std::queue<std::tuple<std::string, int, int, int, char >> currentPlayers) {
	int TopScore = 0;
	std::vector<std::string> winner;
	while (!currentPlayers.empty())
	{
		//std::cout << "player  " << std::get<0>(currentPlayers.front()) << " has Score: " << std::get<1>(currentPlayers.front()) << '\n' << std::endl;
		if (TopScore < std::get<1>(currentPlayers.front()))
		{	
			//keep track of highest score so far
			TopScore = std::get<1>(currentPlayers.front());
			//new high score, clear the vector
			winner.clear();
			//add the player with current high score as the winner
			winner.push_back(std::get<0>(currentPlayers.front()));
		}
		//multiple players with same score
		else if (TopScore == std::get<1>(currentPlayers.front())) {
			//add them to the vector
			winner.push_back(std::get<0>(currentPlayers.front()));
		}
		else {
		}
		currentPlayers.pop();
	}
	//std::cout << "The winner is " << winner << ": " << TopScore;
	return winner;
}




int main()
{
	//configuration variables: declaration

	//userpath for the playArea file- should be loaded from a file
	//wchar_t playAreaPath[]=L"C:\\Users\\Dexter\\Desktop\\testStuff\\stuff.txt";
	wchar_t* playAreaPath = new wchar_t[200];
	//userpath for the folder with bot .txt files- should be loaded from a file
	static std::string userPath;
	//maximum amount of raises loaded from config file
	static int maxRaises;
	//ante, called from file
	static int ante ;
	//small blind, called from file
	static int smallBlind;
	//big blind, called from file
	static int bigBlind;
	//number of players (loaded from config file)	
	static int players;
	//template for time given to bet, will be loaded via config file and will be adjusted as the hand progresses
	static int time_;
	//how big the monte carlo simulation should be- loaded from config file
	static int monteCarloLength;
	//how many iterations the Score should be backed up- 
	static int backupScores;
	//path to the file that stores the backup
	static std::string backupPath;


	//Ask user for the configuration file location
	std::string configPath;
	std::cout << "Please enter the path to the configuration file "<<std::endl<< "in the following format: C:/Users/x..../config.txt" <<std::endl;
	std::getline(std::cin, configPath);
	std::cout<<"Path given: " << configPath<<std::endl;
//* //debugging tool
	if (configPath=="")
	{
		std::cout << "No path given, choosing default "<<std::endl;
		configPath = "C:/Users/Dexter/Desktop/testStuff/config/config.txt";
	}
//*/
	//declare the configuration vector
	std::vector<std::pair<std::string, std::string>> configuration;
	//set the configuartion vector by reading the supplied configuration path
	configuration=readFromConfigFile(configPath);
	//loading in the configuration - loopubg through the entire vector
	for (auto it = configuration.begin(); it != configuration.end(); it++)
	{
		// To get hold of the class pointers:
		//name of the variable
		auto pClass1 = it->first;
		//value of the variable
		auto pClass2 = it->second;
		//settubg the playArea path
		if (pClass1 == "playAreaPath") {
			std::cout << "Setting playAreaPath" << pClass2<<std::endl;
			std::string temp = pClass2;
			//converting to wise string for conversion to win32 standard
			std::wstring wide_string = std::wstring(temp.begin(), temp.end());
			//converting to LPCWSTR to use the lstrcpyW method of win32
			LPCWSTR sw = wide_string.c_str();
			lstrcpyW(playAreaPath, sw);
			
		}
		//setting the user path
		else if (pClass1 == "userPath") {
			userPath = pClass2;
		}
		//setting up the max raises
		else if (pClass1 == "maxRaises") {
			maxRaises = std::stoi(pClass2);
		}
		//setting up the ante
		else if (pClass1 == "ante") {
			ante = std::stoi(pClass2);
		}
		//setting up the small blind
		else if (pClass1 == "smallBlind") {
			smallBlind = std::stoi(pClass2);
		}
		//setting up the big blind
		else if (pClass1 == "bigBlind") {
			bigBlind = std::stoi(pClass2);
		}
		//settting up the amount of players
		else if (pClass1 == "players") {
			players = std::stoi(pClass2);
		}
		//setting up the timeout time
		else if (pClass1 == "time_") {
			time_ = std::stoi(pClass2);
		}
		//setting up the amount of iterations of the simulation
		else if (pClass1 == "monteCarloLength") {
			monteCarloLength= std::stoi(pClass2);
		}
		//setting up how often should the scores be backed up
		else if (pClass1 == "backupScores") {
			backupScores = std::stoi(pClass2);
		}
		//setting up the path to the backup file
		else if (pClass1 == "backupPath") {
			backupPath= pClass2;
		}
		//none of the above, notify user
		else {
			std::cout << "something wrong -illegal variable-value combination";
		}
	}


		//create the read only file
		createReadOnlyFile(playAreaPath);
	
		//Magic numbers, constants, etc
		//number of cards that are the board (community cards)
		static constexpr int boardCards = 5;
		//number of cards that are dealt to each player( hand cards)
		//CHANGING THIS REQUIRES RE-IMPLEMENTING CARD DEALING, DO SO AT OWN RISK
		static constexpr int playerCards = 2;
		//size of a deck of cards
		static constexpr int deckSize = 52;
		
	
		// Create a Hand from a card. CardIdx is an integer between 0 and 51, so that CARD = 4 * RANK + SUIT, where
		// rank ranges from 0 (deuce) to 12 (ace) and suit is from 0 (spade) to 3 (diamond).
		/*****Pre-Game Setup********/
		static const std::string Suit[4]{ "Spades", "Clubs", "Hearts", "Diamonds" };
		static const std::string Rank[13]{ "Deuce", "Three", "Four", "Five","Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King", "Ace" };
		

		//random number generator initlasation: Mersenne Twister with time seed
		//perhaps change the seed after a certain number of iterations?
		std::mt19937 rng(time(0));

		//initalise the deck
		std::vector<int> deck(deckSize);
		std::iota(deck.begin(), deck.end(), 0);
		//deck that will be used in simulation
		std::vector<int> deck_(deckSize);

		//initalise card vector used in simulation
		std::vector<int> cards;

		//fetches all txt files from directory, once
		std::vector<std::string> playerFiles;
		//getting all the .txt files in a directory, and checking if amount of files >=players
		while (true)
		{
			std::vector<std::string> allFiles = fetchTxtFromDirectory(userPath);
			if (allFiles.size() >= players)
			{
				//fetches txt files corresponding to number of players
				playerFiles = fetchTxtFromDirectory(userPath, players);
				break;
			}
		}
		//shuffle the array of players so that they don't always sit in the same order across multiple simulations
		std::shuffle(playerFiles.begin(), playerFiles.end(), rng);

		//create a vector of pairs, which will be used to store the winnings of each player throughout the simulation
		//string stores player name, double stores player score
		std::vector<std::pair<std::string, double>> score;

		/*making the queue of players based on the names of the .txt files they created
		//1st value: player name
		//2nd and 3rd: stores hands- -1 is used as default
		//4th: stores how much the player has bet in current hand- ante is deafult- entry fee to play each round
		//5th: keeps track of which player is the dealer(holds the button) p=player, b= button (t is used as temporary dealer, to deal with the dealer folding)
		*/
		std::queue<std::tuple<std::string, int, int,int,char>> playerQueue;
		for (int i = 0; i < playerFiles.size(); i++)
		{
			//add players to the queue
			std::string playerName = nameFromPath(playerFiles, i);
			auto q = std::make_tuple(playerName, -1, -1,ante,'p');
			//add players to the Score vector
			score.push_back(std::pair<std::string,double> (playerName, 0.0));
			playerQueue.push(q);
		}

		//create a copy of the plater queue, to be used as a backup of the playerQueue
		std::queue<std::tuple<std::string, int, int,int,char>> playerCopy(playerQueue);
		//wait for user to start the simulation
		do
		{
		std::cout << '\n' << "Setup complete, press any button to continue" << std::endl;
		} while (std::cin.get() != '\n');
		


		/********Monte Carlo Simulation*************/
		//

		//to be used for bechmark: measures the time it takes to run monteCarloLength simulations
		std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

		//main game loop
		for (int monteCarloState = 1; monteCarloState <= monteCarloLength; monteCarloState++) {
			//inform players what iteration they are on:
			appendToReadOnlyFile(playAreaPath, &("Current Hand:" + std::to_string(monteCarloState) + "\r\n")[0u]);

			//copy deck for current hand
			deck_ = deck;
			//clear the cards
			cards.clear();

			//used as a reset for the card deletion
			//since the range is from 0-51, counter has to be one less than deckSize
			int counter = deckSize - 1;
			//each player gets 2 cards
			//boardCards- cards that are dealt on the board
			//2- number of cards a player gets
			while (cards.size() != players *  playerCards + boardCards)
			{
				std::uniform_int_distribution<int> distribution(0, counter);
				int number = distribution(rng);
				cards.push_back(deck_[number]);
				//Number represents a cardId, which is is an integer between 0 and 51, such that that CARD = 4 * RANK + SUIT, where
				// rank ranges from 0 (deuce) to 12 (ace) and suit is from 0 (spade) to 3 (diamond).
				int suit = deck_[number] % 4;
				int rank = deck_[number] / 4;
				//debug stuff
				//TestVec.push_back(Rank[rank] + " of " + Suit[suit]);

				if (cards.size() == 1)std::cout << "board cards: \n";
				if (cards.size() <= boardCards)
				{
					std::cout << Rank[rank] << " of " << Suit[suit] << '\n';
				}
				//if number== counter, counter --; will shorten the range by one such that deletion is not neccessery
				if (number != counter)
				{
					deck_.erase(deck_.begin() + number);
				}
				counter--;
			}

			//copy the player queue
			playerCopy = playerQueue;
			//counter to count player's positions 
			int positionCounter = 1;
			//initalising the position message
			std::string positionAllocationMessage;
			//loop to assign cards to all players
			for (int i = 0; i < playerCards * players; i++)
			{


				//boardCards- first x cards are always the board cards (0-(x-1_), thus start from boardCards
				std::get<1>(playerCopy.front()) = cards[boardCards + i];
				//second card
				std::get<2>(playerCopy.front()) = cards[boardCards + i + 1];


				//add one more to the counter, so that there is no overlap in given cards
				i++;
				//write position of players that are not the dealer to playArea
				if (positionCounter != 1) {
					//message to be appended to the read-only file
					positionAllocationMessage = std::get<0>(playerCopy.front()) + " sits at positon " + std::to_string(positionCounter) + "\r\n";
				}
				//if the player is the dealer, assign them a 'b' tag and declare so in the read-only ile
				else {
					//message to be appended to the read-only file
					positionAllocationMessage = std::get<0>(playerCopy.front()) + " sits at positon " + std::to_string(positionCounter) + " [BUTTON]" + "\r\n";
					//setting the player tag to the button/dealer tag
					std::get<4>(playerCopy.front()) = 'b';
				}
				//use append such that data is not overwritten
				appendToReadOnlyFile(playAreaPath, &positionAllocationMessage[0u]);

				//writing dealt cards to playerFile
				writeToFile(userPath + "/" + std::get<0>(playerCopy.front()) + ".txt", std::to_string(std::get<1>(playerCopy.front())) + " " + std::to_string(std::get<2>(playerCopy.front())));
				positionCounter++;
				//add the player to the back of the queue
				playerCopy.push(playerCopy.front());
				//pop player that has been assigned cards
				playerCopy.pop();
			}

			//set the playerQueue to the value of Copy
			playerQueue = playerCopy;

			//displaying the cards recieved to the user via standard output- debugging
			for (int temp=0;temp <playerCopy.size();temp++)
			{


				//Number stored represents a cardId, which is is an integer between 0 and 51, such that that CARD = 4 * RANK + SUIT, where
				// rank ranges from 0 (deuce) to 12 (ace) and suit is from 0 (spade) to 3 (diamond).
				//suit and rank of the first card
				int suit = std::get<1>(playerCopy.front()) % 4;
				int rank = std::get<1>(playerCopy.front()) / 4;
				//suit and rank of the second card
				int suit_ = std::get<2>(playerCopy.front()) % 4;
				int rank_ = std::get<2>(playerCopy.front()) / 4;
				std::cout << "player " << std::get<0>(playerCopy.front()) << " recieved cards " << Rank[rank] << " of " << Suit[suit] << " and " << Rank[rank_] << " of " << Suit[suit_] << "\n";
				playerCopy.push(playerCopy.front());
				playerCopy.pop();
			}



			//betting structure
			//declare and reset the player decision
			std::string decision = "";
			//set the current betting rules for small blind- pre flop and post flop, changes in betting loop once it is post turn and post river 
			int currentBlind = smallBlind;
			//temporary betting queue, which will keep track of players left to made a decision
			std::queue<std::tuple<std::string, int, int, int, char>>bettingQueue(playerQueue);

			//begin new thread for hand evaluation
			auto future = std::async(evaluatePlayerHands, boardCards, cards, playerCopy);

			//integer that will keep track of the bets throughout the game- starts at one as all players bet ante at the start of the game
			int currentBet = ante;
			//this is the Pot contribution from players who folded
			int foldPot = 0;
			//loop for the betting strucure- break; can be used to exit it prematurely, eg. everyone but 1 person folds
			for (int currentState = 0; currentState < 4; currentState++) {
				//small blind rules in first 2 betting rounds, big blind in the other 2 rounds- default is small blind
				if (currentState % 2 == 1)
				{
					currentBlind = bigBlind;
				}
				//integer that will keep track of the amount of raises made in current betting cycle
				int currentRaises = 0;
				//decision counter counts the number of consecutive decisions made by players, to ensure that all players check/called, folded, or where the ones to raise.
				int decisionCounter = 0;

				//pre flop state only
				if (currentState == 0) {
					//this has already been set as a default for each of the players tuple, as such only need to notify them about it
					appendToReadOnlyFile(playAreaPath, &("All players post an ante of " + std::to_string(ante) + "\r\n")[0u]);
				}
				//only 1 player in the game
				if (bettingQueue.size() == 1) {
					std::cout << "Error, the amount of players playing is too small" << std::endl;
					//currently exit for troubleshooting reasons
					std::_Exit(EXIT_FAILURE);
				}

				//betting rules when there are 2 or more players left in the game
				if (bettingQueue.size() >= 2) {
					//case when only 2 players are playing:
					//this only applies when there are 2 bots in the game, and when it's pre-flop
					//rules, as per wikipedia
					//When only two players remain, special 'head-to-head' or 'heads up' rules are enforced
					//and the blinds are posted differently. In this case, the person with the dealer button posts
					//the small blind, while his/her opponent places the big blind. The dealer acts first before the
					//flop. After the flop, the dealer acts last and continues to do so for the remainder of the hand
					if (bettingQueue.size() == 2 && currentState == 0) {
						//dealer posts the small blind
						appendToReadOnlyFile(playAreaPath, &(std::get<0>(bettingQueue.front()) + " posts the small blind: " + std::to_string(smallBlind) + "\r\n")[0u]);
						std::get<3>(bettingQueue.front()) += smallBlind;
						bettingQueue.push(bettingQueue.front());
						bettingQueue.pop();
						//player after dealer posts the big blind
						appendToReadOnlyFile(playAreaPath, &(std::get<0>(bettingQueue.front()) + " posts the big blind: " + std::to_string(bigBlind) + "\r\n")[0u]);
						//	std::get<3>(bettingQueue.front()) += bigBlind;
						std::cout << "check big bet " << std::get<3>(bettingQueue.front());
						currentBet += bigBlind;
						bettingQueue.push(bettingQueue.front());
						bettingQueue.pop();

					}
					//this applies for both 2 player-only games(after the flop) and more than 2 player games (for all betting rounds)
					else {

						//push the dealer to the back- they make the decision last
						bettingQueue.push(bettingQueue.front());
						bettingQueue.pop();
						//player after dealer posts small blind
						appendToReadOnlyFile(playAreaPath, &(std::get<0>(bettingQueue.front()) + " posts the small blind: " + std::to_string(smallBlind) + "\r\n")[0u]);
						std::get<3>(bettingQueue.front()) = std::get<3>(bettingQueue.front()) + smallBlind;
						std::cout << "player: " << std::get<0>(bettingQueue.front()) << " posts small blind: " << smallBlind << "\n";
						bettingQueue.push(bettingQueue.front());
						bettingQueue.pop();
						//2nd player after dealer posts the big blind
						appendToReadOnlyFile(playAreaPath, &(std::get<0>(bettingQueue.front()) + " posts the big blind: " + std::to_string(bigBlind) + "\r\n")[0u]);
						std::get<3>(bettingQueue.front()) = (std::get<3>(bettingQueue.front()) + bigBlind);
						std::cout << "player: " << std::get<0>(bettingQueue.front()) << " posts big blind: " << bigBlind << "\n";
						currentBet += bigBlind;
						bettingQueue.push(bettingQueue.front());
						bettingQueue.pop();
					}
				}

			//using a label with goto: this is used to stay in the same game state while going through the queue of players
			label:
				//all other players folded
				if (bettingQueue.size() == 1)
				{

					std::cout << std::get<0>(bettingQueue.front()) << " won " << std::to_string(foldPot + std::get<3>(bettingQueue.front())) << std::endl;
					
					//update players running total - deduct their current bet from their running total
					for (auto it = score.begin(); it != score.end(); it++)
					{
						// To get hold of the class pointers:
						auto pClass1 = it->first;
						if (pClass1 == std::get<0>(bettingQueue.front()))
						{
							auto pClass2 = it->second;
							//how much the player won = the amount the players who folded bet. Do not include their own contribution to the pot, as it has not been taken away from their running total
							it->second = pClass2 + foldPot;
							std::cout << pClass1 << " total " << it->second << std::endl;
							break;
						}
					}


					//update players running total - add the pot to their running total
					// do include their own contributuion here, since they technically won that much
					appendToReadOnlyFile(playAreaPath, &(std::get<0>(bettingQueue.front()) + " won " + std::to_string(foldPot + std::get<3>(bettingQueue.front())) + ", all other players folded \r\n")[0u]);
					//set the current state to 4 so that a new hand starts after the below code is executed
					currentState = 4;
					//go to the part of the code that deals with score allocation, and backing up the score in a file
					goto jump;
				}
				//if all players have made their decisions for the current round
				if (decisionCounter == bettingQueue.size())
				{

					switch (currentState) {
						//case 0 is pre fold- show fold at the end of the pre-fold phase
					case 0:
						std::cout << "*** FLOP *** [" << cards[0] << " " << cards[1] << " " << cards[2] << "]\n";
						appendToReadOnlyFile(playAreaPath, &("*** FLOP *** [" + std::to_string(cards[0]) + " " + std::to_string(cards[1]) + " " + std::to_string(cards[2]) + "]" + "\r\n")[0u]);
						break;
						//case 1 is pre-turn- show turn at the end of the pre-turn phase
					case 1:
						std::cout << "*** TURN *** [" << cards[0] << " " << cards[1] << " " << cards[2] << "]" << " [" << cards[3] << "]\n";
						appendToReadOnlyFile(playAreaPath, &("*** TURN *** [" + std::to_string(cards[0]) + " " + std::to_string(cards[1]) + " " + std::to_string(cards[2]) + " " + std::to_string(cards[3]) + "]"+"\r\n")[0u]);
						break;
						//case 2 is pre-river - show  river at the end of the pre-river phase
					case 2:
						std::cout << "*** RIVER *** [" << cards[0] << " " << cards[1] << " " << cards[2] << " " << cards[3] << "]" << " [" << cards[4] << "]\n";
						appendToReadOnlyFile(playAreaPath, &("*** RIVER ***[" + std::to_string(cards[0]) + " " + std::to_string(cards[1]) + " " + std::to_string(cards[2]) + " " + std::to_string(cards[3]) + " " + std::to_string(cards[4]) + "]" + "\r\n")[0u]);
						break;
						//case 3 is post-river- declare showdown
					case 3:
						std::cout << ("*** SHOWDOWN ***\n");
						appendToReadOnlyFile(playAreaPath, "*** SHOWDOWN *** \r\n");
						break;


					}
					//debugging- showing the ante
					std::cout << currentBet - ante<< " This is the bet the betting round ended with" << std::endl;
					//debugging- showing the pot at the end of the round
					std::cout << currentBet*bettingQueue.size() + foldPot << " This is the current pot at end of the betting round" << std::endl;



					//loop through queue until the dealer is again in the first position
					//this means if current player has tag 'p', they are not a dealer, as such, advance the queue
					while (std::get<4>(bettingQueue.front()) == 'p')
					{
						//debugging, shows that the queue is being rearranged
						std::cout << "rearanging queue" << std::endl;
						bettingQueue.push(bettingQueue.front());
						bettingQueue.pop();
					}
					//move to the next betting round
					if (currentState == 3)
					{
						//go to the part of the code that deals with score allocation, and backing up the score in a file
						goto jump;
					}
					//move on to the next betting round
					continue;


				}
				//time-out clock
				clock_t start;
				clock_t endwait;
				start = clock();
				//the time given to each player is adjusted by what betting state it's in- pre-flop it has 1/4 of the time, post flop 2/4, post-turn 3/4 etc
				endwait = start + (time_*(.25*(currentState + 1))*1000); //times 1000 to turn it into milliseconds
				//checking if decision was made
				boolean decisionFlag = false;
				//if there are still players playing and the current one has not timed out
				while (!bettingQueue.empty() && clock() < endwait) {
					//read the decision of the user
					decision = readFromFile(userPath + "/" + std::get<0>(bettingQueue.front()) + ".txt");
					//converting the decision to a char- ensure that the file contains a single character, or that your decison is the first character in the file
					char *i = &decision[0u];
					//switch case, deciding what the player decision was
					switch (i[0]) {
					//Player folded
					case 'f':

						std::cout << std::get<0>(bettingQueue.front()) << " folded " << std::get<3>(bettingQueue.front()) << std::endl;
						appendToReadOnlyFile(playAreaPath, &(std::get<0>(bettingQueue.front()) + " folded \r\n")[0u]);
						//they made a decision, set the decision flag to true
						decisionFlag = true;
						//update players running total - deduct their current bet from their running total
						for (auto it = score.begin(); it != score.end(); it++)
						{
							// To get hold of the class pointers:
							auto pClass1 = it->first;
							if (pClass1 == std::get<0>(bettingQueue.front()))
							{
								auto pClass2 = it->second;
								it->second = pClass2 - std::get<3>(bettingQueue.front());
								std::cout << pClass1 << " total " << it->second << std::endl;
								break;
							}
						}

						//increase the foldpot
						foldPot += std::get<3>(bettingQueue.front());
						std::cout << "current pot " << foldPot << std::endl;
						//overwrite their message such that they have time to make their next decision, in case the next betting round stands right after they fold
						writeToFile(userPath + "/" + std::get<0>(bettingQueue.front()) + ".txt", "");
						//check if the folded player was a dealer or a temporary, if so, set the next player to be temporary dealer
						if (std::get<4>(bettingQueue.front()) == 'b' || std::get<4>(bettingQueue.front()) == 't' && !bettingQueue.empty())
						{

							//std::cout << "player " << std::get<0>(bettingQueue.front()) << " folded"<<std::endl;
							//pop current player
							bettingQueue.pop();
							//set following player to be the temporary dealer
							std::get<4>(bettingQueue.front()) = 't';
							std::cout << "player " << std::get<0>(bettingQueue.front()) << " is temporary dealer" << std::endl;
						}
						//otherwise they were not a dealer player
						else {
							bettingQueue.pop();
						}

						goto label;
						break;
					//Player raised
					case 'r':
						//check if they did not go over the raise limit
						if (currentRaises != maxRaises)
						{
							std::cout << std::get<0>(bettingQueue.front()) << " raised" << std::endl;
							//increase the current bet
							currentBet += currentBlind;
							appendToReadOnlyFile(playAreaPath, &(std::get<0>(bettingQueue.front()) + " raised: " + std::to_string(currentBlind) + ", to: " + std::to_string(currentBet-ante) + "\r\n")[0u]);
							//update the players bet
							std::get<3>(bettingQueue.front()) = currentBet;
							//player made a decision, set the flag to true
							decisionFlag = true;
							//overwrite their message such that they have time to make their next decision, if an opponent makes a decision immediately
							writeToFile(userPath + "/" + std::get<0>(bettingQueue.front()) + ".txt", "");
							//move them to the back of the queue
							bettingQueue.push(bettingQueue.front());
							bettingQueue.pop();
							currentRaises++;
							//set decision counter to 1 after a raise
							decisionCounter = 1;
							//go to the start of the queue iteration code
							goto label;
						}
						//they did raise above the limit- auto fold
						else {
							std::cout << std::get<0>(bettingQueue.front()) << " raised past limit- Auto fold" << std::endl;
							foldPot += std::get<3>(bettingQueue.front());
							//overwrite their message such that they have time to make their next decision,  in case the next betting round stands right after they fold
							writeToFile(userPath + "/" + std::get<0>(bettingQueue.front()) + ".txt", "");
							appendToReadOnlyFile(playAreaPath, &(std::get<0>(bettingQueue.front()) + " raised over the betting limit, auto-fold \r\n")[0u]);

							//update players running total - deduct their current bet from their running total
							for (auto it = score.begin(); it != score.end(); it++)
							{
								// To get hold of the class pointers:
								auto pClass1 = it->first;
								if (pClass1 == std::get<0>(bettingQueue.front()))
								{
									auto pClass2 = it->second;
									it->second = pClass2 - std::get<3>(bettingQueue.front());
									std::cout << pClass1 << " total " << it->second << std::endl;
									break;
								}
							}

							//check if the folded player was a dealer/temporary dealer, if so, set the next player to be temporary dealer
							if (std::get<4>(bettingQueue.front()) == 'b' || std::get<4>(bettingQueue.front()) == 't')
							{
								//pop current player
								bettingQueue.pop();
								//set following player to be the temporary dealer
								std::get<4>(bettingQueue.front()) = 't';
							}
							else {
								bettingQueue.pop();
							}
							goto label;
						}

						break;
					//the player checked/called
					case 'c':
						//player needs to call to stay in the game if their bet is less than current, higher bet
						if (std::get<3>(bettingQueue.front()) < currentBet) {
							appendToReadOnlyFile(playAreaPath, &(std::get<0>(bettingQueue.front()) + " called: " + std::to_string(currentBet - std::get<3>(bettingQueue.front())) + ", to: " + std::to_string(currentBet-ante) + "\r\n")[0u]);
							std::get<3>(bettingQueue.front()) = currentBet;
							std::cout << std::get<0>(bettingQueue.front()) << " called" << std::endl;

							//overwrite their message such that they have time to make their next decision, if an opponent makes a decision immediately
							writeToFile(userPath + "/" + std::get<0>(bettingQueue.front()) + ".txt", "");
							//add one to counter after a call
							decisionCounter++;
							//move player to the back of the queue
							bettingQueue.push(bettingQueue.front());
							bettingQueue.pop();
							//go to the start of the queue iteration code
							goto label;
							break;
						}
						//else their bet =current bet
						else {
							std::cout << std::get<0>(bettingQueue.front()) << " checked" << std::endl;
							appendToReadOnlyFile(playAreaPath, &(std::get<0>(bettingQueue.front()) + " checked \r\n")[0u]);
							//overwrite their message such that they have time to make their next decision, if an opponent makes a decision immediately

							writeToFile(userPath + "/" + std::get<0>(bettingQueue.front()) + ".txt", "");
							//add one to counter after a check
							decisionCounter++;
							bettingQueue.push(bettingQueue.front());
							bettingQueue.pop();
							goto label;
							break;
						}
					}
				}
				//the player made no decision, there are still players left in the game, and the above timer has ran out
				if (decisionFlag == false && !bettingQueue.empty())
				{
					std::cout << std::get<0>(bettingQueue.front()) << " made no decision- Auto fold " << std::endl;
					appendToReadOnlyFile(playAreaPath, &(std::get<0>(bettingQueue.front()) + " made no decision- Auto fold \r\n")[0u]);
					//increase the fold pot
					foldPot += std::get<3>(bettingQueue.front());
					//overwrite their message such that they have time to make their next decision, in case the next betting round stands right after they fold
					writeToFile(userPath + "/" + std::get<0>(bettingQueue.front()) + ".txt", "");

					//update players running total - deduct their current bet from their running total
					for (auto it = score.begin(); it != score.end(); it++)
					{
						// To get hold of the class pointers:
						auto pClass1 = it->first;
						if (pClass1 == std::get<0>(bettingQueue.front()))
						{
							auto pClass2 = it->second;
							it->second = pClass2 - std::get<3>(bettingQueue.front());
							std::cout << pClass1 << " total " << it->second << std::endl;
							break;
						}
					}


					//check if the folded player was a dealer or a temporary, if so, set the next player to be temporary dealer
					if (std::get<4>(bettingQueue.front()) == 'b' || std::get<4>(bettingQueue.front()) == 't' && !bettingQueue.empty())
					{
						//pop current player
						bettingQueue.pop();
						//set following player to be the temporary dealer
						std::get<4>(bettingQueue.front()) = 't';
					}
					else {
						//not dealer or temporary dealer, pop them
						bettingQueue.pop();
					}
					//go to the start of the queue iteration code
					goto label;
				}


			}
			
		//the part of the code that deals with score allocation, and backing up the score in a file
		jump:
			//get data from thread running the evaluation
			playerCopy = future.get();

			//for showdown-if at the end of the game there are 2 or more players remaining
			if (bettingQueue.size() >= 2)
			{
				for (int i = 0; i < bettingQueue.size(); i++)
				{
					while (true) {
						//need to ensure that only players still in the game are the ones that will get their score considered
						//thus, compare the queue of players still in the game with the queue of players that started in the game, and get their hand+board scores
						if (std::get<0>(bettingQueue.front()) == std::get<0>(playerCopy.front())) {
							std::get<1>(bettingQueue.front()) = std::get<1>(playerCopy.front());
							//Move the player to the end of the player queue
							bettingQueue.push(bettingQueue.front());
							bettingQueue.pop();
							//remove them from the playercopy 
							playerCopy.pop();
							break;
						}
						else {
							//push player to the end
							playerCopy.push(playerCopy.front());
							playerCopy.pop();
						}
					}
				}
				
				std::vector<std::string> winner = declareWinner(bettingQueue);
				//only one winner
				if (winner.size() == 1)
				{
					appendToReadOnlyFile(playAreaPath, &(winner[0] + " won \r\n")[0u]);
					std::cout << "Winner is " << winner[0] << std::endl;
					//update players running total - deduct their current bet from their running total
					for (auto it = score.begin(); it != score.end(); it++)
					{
						// To get hold of the class pointers:
						auto pClass1 = it->first;
						if (pClass1 == winner[0])
						{
							auto pClass2 = it->second;
							it->second = pClass2 + foldPot + (currentBet * (bettingQueue.size() - 1));
							std::cout << pClass1 << " total " << it->second << std::endl;
							break;
						}
					}




				}
				//more than one winner
				else {
					std::string winners = "";
					for (int w = 0; w < winner.size(); w++) {
						//more than one winners remain
						if (winner.size() > w)
						{
							winners += winner[w] + ", ";
						}
						//last winner
						else {
							winners += winner[w];
						}
					}
					//allocate winnings
					double splitWinnings = (foldPot + (currentBet * (bettingQueue.size()) - winner.size())) / winner.size();

					for (int a = 0; a < winner.size(); a++) {
						//update players running total - deduct their current bet from their running total
						for (auto it = score.begin(); it != score.end(); it++)
						{
							// To get hold of the class pointers:
							auto pClass1 = it->first;
							if (pClass1 == winner[a])
							{
								auto pClass2 = it->second;
								it->second = pClass2 + splitWinnings;
								std::cout << pClass1 << " total " << it->second << std::endl;
								break;
							}
						}
					}
					for (int winners = 0; winners < winner.size(); winners++) {
					appendToReadOnlyFile(playAreaPath, &(winner[winners] + " won" + "\r\n")[0u]);
					std::cout << "Winner: " << winner[winners] << std::endl;
					}
					//std::_Exit(EXIT_FAILURE);

				}
			}
			//go-to label necessery as a prevention measure of the continue
			
			//for displaying everyone's hand 
			std::cout << "Board cards: [" << cards[0] << ", " << cards[1] << ", " << cards[2] << ", " << cards[3] << ", " << cards[4] << "]" <<"\n";
			appendToReadOnlyFile(playAreaPath, &("Board cards: [" + std::to_string(cards[0]) + ", " + std::to_string(cards[1]) + ", " + std::to_string(cards[2]) + ", " + std::to_string(cards[3]) + ", " + std::to_string(cards[4]) + "]"+"\r\n")[0u]);

			for (int i = 0; i < playerQueue.size(); i++)
			{
				std::cout << std::get<0>(playerQueue.front()) << " had cards [" << std::get<1>(playerQueue.front()) << ", " << std::get<2>(playerQueue.front()) << "]" << std::endl;
				appendToReadOnlyFile(playAreaPath, &(std::get<0>(playerQueue.front()) + " had cards [" + std::to_string(std::get<1>(playerQueue.front())) + ", " + std::to_string(std::get<2>(playerQueue.front())) + "]"+"\r\n")[0u]);
				//push player to the end
				playerQueue.push(playerQueue.front());
				playerQueue.pop();
			}


			//save the winnings of each player periodically
			if (monteCarloState % backupScores == 0) {
				std::string backupTemp;
				backupTemp += ("Iteration: "+ std::to_string(monteCarloState)  + "\n");
				for (int a = 0; a < score.size(); a++) {
					backupTemp += (score[a].first + ", " + std::to_string(score[a].second)) + "\n";
					
				}
				writeToFile(backupPath, backupTemp);
			}
			//re-arrange queue so that dealer==last
			playerQueue.push(playerQueue.front());
			playerQueue.pop();
			//sleep for waittime to allow players to read the playAreaFile - go from seconds to milliseconds
			std::this_thread::sleep_for(std::chrono::milliseconds(time_ * 1000));
			//reset the content of the read-only file
			createReadOnlyFile(playAreaPath);
		
			
		}
typedef std::chrono::duration<float> fsec;
std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
fsec fs= t2 - t1;
appendToFile(backupPath,std::to_string(fs.count())+ " s");
std::cout << fs.count()<<" seconds"<<std::endl;

//make the temporary best score equal to the score of the player at position 0
double temp = score[0].second;
//make the string holding the best player's name the player at position 0
std::string bestScore = score[0].first;
	for (int a = 1; a < score.size(); a++) {
		//current player has higher score
		if (temp < score[a].second) {
			temp = score[a].second;
			bestScore = score[a].first;
		}
		//2 players have the exact same score
		else if (temp == score[a].second) {
			bestScore += score[a].first;
		}
		else {

		}
	}
std::cout << "The winner of the simulation is: " << bestScore << " with the score of: " << std::to_string(temp)<<std::endl;
std::cout << "Simulation complete, press any key to exit" << std::endl;
std::getline(std::cin, configPath);

	return 0;
}
