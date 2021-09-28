#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "bufferPool.h"
#include "bpTree.cpp"

using namespace std;
typedef unsigned int uint;

int main()
{
	uint blkSize, n;
	string line;
	cout << "Enter the block size (Bytes): ";
	cin >> blkSize;

	// 200 MB of memory allocated
	bufferPool bufferPool{200000000, blkSize};
	vector<Address> dataset;
	cout << "Reading data from data file..." << endl;

	ifstream dataFile("data.tsv");
	if (dataFile.is_open())
	{
		// Skip the first line
		getline(dataFile, line);

		while (getline(dataFile, line))
		{
			Record record;
			Address dataRecord;
			string tempData;

			// Get the data of tconst before the tab delimiter
			strcpy_s(record.tconst, SIZE, line.substr(0, line.find('\t')).c_str());

			// Get individual data seperated by space after the tab delimiter
			stringstream ss(line);
			getline(ss, tempData, '\t');
			ss >> record.avgRating >> record.numVotes;

			dataRecord = bufferPool.writeRecordToBlk(sizeof(record));
			dataset.push_back(dataRecord);

			void* recordAddress = (uchar*)dataRecord.blockAddress + dataRecord.offset;
			memcpy(recordAddress, &record, sizeof(record));
		}
		dataFile.close();
	}
	else cout << "Unable to open/detect data.tsv file.\n";

	cout << endl << "------------------------Experiment 1------------------------" << endl << endl;
	cout << "Number of blocks = " << bufferPool.getNumOfBlkAlloc() << endl;
	cout << "Size of Database = " << double(bufferPool.getTotalRecordSize())/(1000*1000) << "MB" << endl;
	cout << "Number of Blocks remaining = " << bufferPool.getNumOfBlkAvail() << endl;

	BPTree tree;
	cout << endl << "------------------------Experiment 2------------------------" << endl << endl;
	for (int i = 0; i < dataset.size(); ++i) {
		void* mainMemoryAddress = (uchar*)dataset[i].blockAddress + dataset[i].offset;
		uint numVotes = (*(Record*)mainMemoryAddress).numVotes;
		tree.insert(dataset[i], numVotes);
	}
	cout << "Parameter (n) of B+ Tree = " << MAX << endl;
	cout << "Number of Nodes of B+ Tree = " << tree.getNumOfNode() << endl;
	cout << "Height of the B+ Tree = " << tree.getTreeLvl() << endl;
	tree.display(tree.getRoot(), 1, 0);
	
	cout << endl << "------------------------Experiment 3------------------------" << endl << endl;
	cout << "Searching for 'numVotes' = 20..." << endl << endl;
	tree.search(20, 20);

	cout << endl << "------------------------Experiment 4------------------------" << endl << endl;
	cout << "Searching for 'numVotes' = 0 to 500..." << endl << endl;
	tree.search(0, 500);
	return 0;
}