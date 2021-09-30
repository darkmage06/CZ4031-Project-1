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
	uint input, blkSize, n;
	string line;
	cout << "Choose the block size (Bytes):" << endl;
	cout << "1. 100 Bytes" << endl;
	cout << "2. 500 Bytes" << endl;
	cout << "0. Exit" << endl;
	cin >> input;
	if (input == 1)
		blkSize = 100;
	else if (input == 2)
		blkSize = 500;
	else
		return 0;

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

	cout << endl << "------------------------Experiment 5------------------------" << endl << endl;
	cout << "Deleting for 'numVotes' = 51..." << endl;
	while (tree.remove(20, &bufferPool));
	//tree.remove(1645, &bufferPool);
	//tree.remove(198, &bufferPool);
	//tree.remove(1342, &bufferPool);
	//tree.remove(120, &bufferPool);
	//tree.remove(2127, &bufferPool);
	//tree.remove(115, &bufferPool);
	//tree.remove(652, &bufferPool);
	//tree.remove(1807, &bufferPool);
	//tree.remove(154, &bufferPool);
	//tree.remove(6018, &bufferPool);
	//tree.remove(262, &bufferPool);
	//tree.remove(10287, &bufferPool);
	//tree.remove(1575, &bufferPool);
	//tree.remove(2850, &bufferPool);
	//tree.remove(888, &bufferPool);
	cout << "Number of Nodes deleted = " << tree.getNumOfNodeDel() << endl;
	cout << "Number of Nodes of B+ Tree = " << tree.getNumOfNode() << endl;
	cout << "Height of the B+ Tree = " << tree.getTreeLvl() << endl;
	tree.display(tree.getRoot(), 1, 0);
	return 0;
}