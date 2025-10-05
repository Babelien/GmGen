#pragma once 

#include <iostream>
#include <fstream>
#include <string>
#include <vector>


struct Header
{
	unsigned char MThd[4];
	unsigned char dataSize[4];
	unsigned char format[2];
	unsigned char trackSize[2];
	unsigned char timeUnit[2];
};

struct Track
{
	unsigned char MTrk[4];
	unsigned char dataSize[4];

	unsigned char instrument;
	std::string trackName;

	std::vector<unsigned char> data;
};

class MidiProcessor
{
public:
	MidiProcessor();
	int importMidi(const char* filePath);
	int exportMidi(const char* filePath, unsigned char instruments[]);
	Header getHeader() const { return _header; }
	Track* getTracks() const { return _tracks; }
	std::vector<std::string> getTrackNames()
	{
		std::vector<std::string> trackNames;
		for (int i = 1; i < carrtoi(_header.trackSize, 2); ++i)
		{
			trackNames.emplace_back(_tracks[i].trackName);
		}

		return trackNames;
	}

private:
	int carrtoi(unsigned char arr[], int size);
	int getDeltaTime(unsigned char data[], int& index);
	int getChannel(int instTrackNumber, unsigned char instruments[]);
	void eventChFix(int roopCounter, int event, int eventNumOfChOne, int index, unsigned char instruments[]);

	Header _header;
	Track* _tracks;

	bool _hasGmResetSysEx;
};

