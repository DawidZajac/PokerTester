#include "stdafx.h"
//#include "PokerLibrary/omp/Hand.h"
//#include "PokerLibrary/omp/OffsetTable.hxx"
//#include "PokerLibrary/omp/Util.h"
//#include "PokerLibrary/omp/Random.h"
#include "PokerLibrary/omp/HandEvaluator.h"
//#include "PokerLibrary/omp/EquityCalculator.h"
//#include "PokerLibrary/omp/Constants.h"
//#include "PokerLibrary/omp/CombinedRange.h"
//#include "PokerLibrary/omp/CardRange.h"
//#include "PokerLibrary/libdivide/libdivide.h"
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


//writing data to a file
void writeToFile(std::string name, std::string text)
{
	std::ofstream myfile;
	myfile.open(name);
	myfile << text;
	myfile.close();
}

//appendiing data to end of file
void appendToFile(std::string name, std::string text)
{
	std::ofstream myfile;
	myfile.open(name, std::ios_base::app);
	myfile << text;
	myfile.close();
}

//reading data from file (debug)
std::string readFromFile(std::string name)
{
	std::ifstream input(name);
	std::string line;

	while (std::getline(input, line)) {
		std::cout<<line << '\n';
	}
	input.close();
	return line;

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
		//detects if the 
		if (*i == '\\')
		{
			return stuffs;
		}
		stuffs.insert(stuffs.begin(), *i);

	}
	return "";

}

//currently fetches the name of the png file - debugging
//fetches the name of the file i.e extract all directory and extension information

std::vector<std::string> fetchTxtFromDirectory(std::string path) 
{
	std::regex rx(".+\\.png$", std::regex::icase);
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
//currently uses ng files for debugging
//might be inefficient, possibly room for optimisation? (check if more files were added before running?)
std::vector<std::string> fetchTxtFromDirectory(std::string path, int players) 
{
	std::regex rx(".+\\.png$", std::regex::icase);
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
		fetchTxtFromDirectory(path, players);
	}
	else
	{
		return i;
	}
}





int main()
{
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
	
		/* monte carlo testing
		std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
		for (int monteCarloCheck = 0; monteCarloCheck <= 100000; monteCarloCheck++)
		{
		//testing if stuff works 
		std::vector<std::string> TestVec;
		std::vector<std::string> TestVec2;
		*/
		//initalise the deck
		std::vector<int> deck_(deckSize);
		std::iota(deck_.begin(), deck_.end(), 0);
		//deck that will be used in simulation
		std::vector<int> deck;

		//initalise card vector used in simulation
		std::vector<int> cards;

		//implement config file
		//number of players (loaded from config file)	
		int players = 8;

		//fetching all txt files from a directory
		//fetches all txt files from directory, once
		//currently pulling all .png files for debugging purposes
		std::vector<std::string> a = fetchTxtFromDirectory("C:/Users/Dexter/Pictures");
		//fetches txt files corresponding to number of players, waiting until there are enough files
		std::vector<std::string> b = fetchTxtFromDirectory("C:/Users/Dexter/Pictures", players);


		//making the queue of players based on the names of the .txt files they created
		//also stores hands- -1 is used as default
		std::queue<std::tuple<std::string, int, int>> playerQueue;
		for (int i = 0; i < b.size(); i++)
		{

			auto q = std::make_tuple(nameFromPath(b, i), -1, -1);
			playerQueue.push(q);
		}


		std::queue<std::tuple<std::string, int, int>> playerCopy(playerQueue);

		//print all elements from a queue (destroys the queue in the process- back it up first!)
		//shows how to access particular elements of a tuple
		//debug use only
		/*
		while (!playerCopy.empty())
		{

			std::cout << std::get<0>(playerCopy.front())<< "\t";
			std::cout << std::get<1>(playerCopy.front()) << "\t";
			std::cout << std::get<2>(playerCopy.front()) << "\n";
			playerCopy.pop();
		}
		*/



		/********Monte Carlo Simulation*************/
		//To push player in 1st to last, push him to end & then pop
		//here is where you would notify about button etc
		//playerCopy.push(playerCopy.front());
		//playerCopy.pop();
		//

		//copy deck for current game
		deck = deck_;

		//clear the cards
		cards.clear();

		//used as a reset for the card deletion
		//since the range is from 0-51, counter has to be one less than deckSize
		int counter = deckSize-1;

		//bebchnark stuff
		//float avg=0.0f;

		//genereting random cards
		//std::count(cards.begin(), cards.end(), players * 2 + 5)!=true
		/*
		//bechmark stuff
		for (int i = 0; i < 10000000; i++) {
			std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
			//*/
			//selecting random cards
			//debug stuff

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
			
			if (cards.size()==1)std::cout << "board cards: \n";
			if (cards.size() <= boardCards)
			{
				std::cout << Rank[rank]<< " of "<< Suit[suit] << '\n';
			}
			//if number== counter, counter --; will shorten the range by one such that deletion is not neccessery
			if (number != counter)
			{
				//std::iter_swap(deck.begin() + number, deck.begin()+counter);
				//chose to erase rather than swap with last, slightly slower but might scew randomness?
				deck_.erase(deck_.begin() + number);
			}
			counter--;
		}
		/*
		//benchmark stuff:
		std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration<float, std::ratio<14 * 24 * 60 * 60, 1000000>>(t2 - t1).count();
		avg += duration;
	}
	std::cout <<avg;
	//*/

	//get player hands from array
	//players[];
	//To push player in 1st to last, push him to end & then pop
	//here is where you would notify about button etc
	//playerCopy.push(playerCopy.front());
	//playerCopy.pop();
	//first card

	//loop to assign cards to all players
		playerCopy = playerQueue;
		for (int i = 0; i <  playerCards * players; i++)
		{
			//boardCards- first x cards are always the board cards (0-(x-1_), thus start from boardCards
			std::get<1>(playerCopy.front()) = cards[boardCards + i];
			//second card
			std::get<2>(playerCopy.front()) = cards[boardCards + i + 1];
			//add the player to the back of the queue
			playerCopy.push(playerCopy.front());
			//pop playert that has been assigned cards
			playerCopy.pop();
			//add one more to the counter, so that there is no overlap in given cards
			i++;
		}

		//testing if it works 
		//counter = 0;
		playerQueue = playerCopy;
		while (!playerCopy.empty())
		{
			//Number stored represents a cardId, which is is an integer between 0 and 51, such that that CARD = 4 * RANK + SUIT, where
			// rank ranges from 0 (deuce) to 12 (ace) and suit is from 0 (spade) to 3 (diamond).
			int suit = std::get<1>(playerCopy.front()) % 4;
			int rank = std::get<1>(playerCopy.front()) / 4;
			int suit_ = std::get<2>(playerCopy.front()) % 4;
			int rank_ = std::get<2>(playerCopy.front()) / 4;

			/*TestVec2.push_back(Rank[rank] + " of " + Suit[suit]);
			TestVec2.push_back(Rank[rank_] + " of " + Suit[suit_]);

			if (TestVec2[counter] != TestVec[counter + 5] || TestVec2[counter + 1] != TestVec[counter + 6])
			{
				std::cout << "Encountered a problem at loop number " <<monteCarloCheck;
				std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
				auto duration = std::chrono::duration<float, std::ratio<14 * 24 * 60 * 60, 1000000>>(t2 - t1).count();
				std::cout << duration;
				return 0;
			}
			else counter + 2;
			*/
			//std::cout << "number : " << number << " counter: " << counter << " number stored: " << deck[number] << " " << Rank[rank] << " of " << Suit[suit] << '\n';
			std::cout << "player "<< std::get<0>(playerCopy.front()) <<" recieved cards " << Rank[rank] << " of " << Suit[suit] << " and " << Rank[rank_] << " of " << Suit[suit_]<< "\n";
			playerCopy.pop();
		}

		//sdasdaasdasasdfadsafds


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
		//template- amount of players would be updated as they fold
		std::queue<std::tuple<std::string, int, int>> currentPlayers(playerQueue);
		//creating hand object for each player still in the game

		for(int i=0;i<currentPlayers.size();i++)
		{
			//Number stored represents a cardId, which is is an integer between 0 and 51, such that that CARD = 4 * RANK + SUIT, where
			// rank ranges from 0 (deuce) to 12 (ace) and suit is from 0 (spade) to 3 (diamond).
			Hand PlayerHand = h;
			PlayerHand += Hand(std::get<1>(currentPlayers.front())) + Hand(std::get<2>(currentPlayers.front()));
			//store the score of the player in the first slot
			std::get<1>(currentPlayers.front())=eval.evaluate(PlayerHand);
			currentPlayers.push(currentPlayers.front());
			currentPlayers.pop();
		}

		while (!currentPlayers.empty())
		{
		std::cout << "player  " << std::get<0>(currentPlayers.front()) << " has Score: "<< std::get<1>(currentPlayers.front()) << '\n' << std::endl;
		currentPlayers.pop();
		}
		//  comment out while dubugging 
		//	std::cout << "player 1: " <<player1 << '\n'<<"player 2: "  << player2 << '\n' << std::endl;
		counter = deckSize;
		//displayVector(cards);
			//TestVec.clear();
			//TestVec2.clear();
	//}

	//file name
	std::string configFile = "config.txt";
	//std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	//auto duration = std::chrono::duration<float, std::ratio<14 * 24 * 60 * 60, 1000000>>(t2 - t1).count();
	//std::cout << duration;
	//writeToFile(configFile,"hello \n");
	//readFromFile(configFile);
	//appendToFile(configFile, "there");
	//readFromFile(configFile);
	//displayVector(deck);
	return 0;
}
//benchmark results:
//generting cards for 4 players + board cards
//evaluating 2 of the 4 player's hands
//monte carlo: 100,000 
//avg time: 16 ms

/*
	auto filename = "/path/to/file";
	struct stat result;
	if (stat(filename.c_str(), &result) == 0)
	{
		auto mod_time = result.st_mtim;
		...
	}


	std::cout << "Hello World!" << std::endl;
	int x;
	std::cout << x << std::endl;
	*/

// to check if file was edited_stat