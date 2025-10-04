#include "pch.h"
#include "MidiProcessor.h"

#include <cmath>

using namespace std;

MidiProcessor::MidiProcessor() :_hasGmResetSysEx(false)
{

}

int MidiProcessor::importMidi(const char* filePath)
{
	ifstream ifs(filePath, ios::in | ios::binary);

	if (!ifs)
	{
		throw runtime_error("Failed to open MIDI file!");
		return 1;
	}

	ifs.read(reinterpret_cast<char*>(&_header.MThd), sizeof(_header.MThd));
	ifs.read(reinterpret_cast<char*>(&_header.dataSize), sizeof(_header.dataSize));
	ifs.read(reinterpret_cast<char*>(&_header.format), sizeof(_header.format));
	ifs.read(reinterpret_cast<char*>(&_header.trackSize), sizeof(_header.trackSize));
	ifs.read(reinterpret_cast<char*>(&_header.timeUnit), sizeof(_header.timeUnit));

	int trackSize = carrtoi(_header.trackSize, 2);
	_tracks = new Track[trackSize];

	for (int i = 0; i < trackSize; ++i)
	{
		//_tracks[i].hasGmResetSysEx = true;
		ifs.read(reinterpret_cast<char*>(_tracks[i].MTrk), sizeof(_tracks[i].MTrk));
		ifs.read(reinterpret_cast<char*>(_tracks[i].dataSize), sizeof(_tracks[i].dataSize));

		_tracks[i].data.resize(carrtoi(_tracks[i].dataSize, 4));
		ifs.read(reinterpret_cast<char*>(_tracks[i].data.data()), carrtoi(_tracks[i].dataSize, 4));

		unsigned char* data = _tracks[i].data.data();

		_tracks[i].instrument = static_cast<unsigned char>(0x00);

		int index = 0;

		while (true)
		{
			int deltaTime = getDeltaTime(data, index);

			int event = static_cast<int>(data[index++]);

			if (event >= 0x80 && event <= 0x8F) // ノートオフ
			{
				index += 2;
			}
			else if (event >= 0x90 && event <= 0x9F) // ノートオン
			{
				index += 2;
			}
			else if (event >= 0xA0 && event <= 0xAF)
			{
				index += 2;
			}
			else if (event >= 0xB0 && event <= 0xBF)
			{
				index += 2;
			}
			else if (event >= 0xC0 && event <= 0xCF) // 音色選択
			{
				//ifs.read(reinterpret_cast<char*>(&_tracks[i].instrument), sizeof(_tracks[i].instrument));
				int midiChannel = event - 0xC0;
				_tracks[i].instrument = midiChannel != 9 ? data[index] : static_cast<unsigned char>(128);
				index++;
			}
			else if (event >= 0xD0 && event <= 0xDF)
			{
				index++;
			}
			else if (event == 0xFF) // メタイベント
			{
				int type = static_cast<int>(data[index++]);

				int dataSize = getDeltaTime(data, index);

				if (type == 0x2F) // トラック終端
				{
					break;
				}

				unsigned char* info = new unsigned char[dataSize];
				for (int j = 0; j < dataSize; ++j)
				{
					info[j] = data[index++];
				}

				if (type == 0x03) // トラック名
				{
					_tracks[i].trackName = string(reinterpret_cast<char*>(info), dataSize);
				}
			}
			else if (i == 0 && event == 0xF0) // システムエクスクルーシブ
			{
				if (static_cast<int>(data[index]) == 0x05 && static_cast<int>(data[index + 1]) == 0x7E && static_cast<int>(data[index + 2]) == 0x7F && static_cast<int>(data[index + 3]) == 0x09 &&
					static_cast<int>(data[index + 4]) == 0x01 && static_cast<int>(data[index + 5]) == 0xF7) // GM Reset
				{
					_hasGmResetSysEx = true;
					index += 6;
				}
			}
			else // その他イベント
			{
				throw runtime_error(" unknown event found!");
				return 1;
			}
		}
	}

	return 0;
}

int MidiProcessor::exportMidi(const char* filePath, unsigned char instruments[])
{
	// 楽器情報の削除
	for (int i = 1; i < carrtoi(_header.trackSize, 2); ++i)
	{
		int index = 0;
		unsigned char* data = _tracks[i].data.data();
		while (index < carrtoi(_tracks[i].dataSize, 4))
		{
			int deltaTime = getDeltaTime(data, index);

			int event = static_cast<int>(data[index++]);

			if (event >= 0x80 && event <= 0x8F) // ノートオフ
			{
				eventChFix(i, event, 0x80, index, instruments[i - 1]);
				index += 2;
			}
			else if (event >= 0x90 && event <= 0x9F) // ノートオン
			{
				eventChFix(i, event, 0x90, index, instruments[i - 1]);
				index += 2;
			}
			else if (event >= 0xA0 && event <= 0xAF)
			{
				eventChFix(i, event, 0xA0, index, instruments[i - 1]);
				index += 2;
			}
			else if (event >= 0xB0 && event <= 0xBF)
			{
				eventChFix(i, event, 0xB0, index, instruments[i - 1]);
				index += 2;
			}
			else if (event >= 0xC0 && event <= 0xCF) // 音色選択
			{
				int start = index - 2;

				_tracks[i].data.erase(_tracks[i].data.begin() + start, _tracks[i].data.begin() + start + 3); // 楽器削除

				int dataSize = carrtoi(_tracks[i].dataSize, 4);

				dataSize -= 3;

				int n = dataSize;
				unsigned char tmp[4] = {};
				for (int j = 0; j < 4; ++j)
				{
					_tracks[i].dataSize[3 - j] = static_cast<unsigned char>(n % (16 * 16));
					n = (n - n % (16 * 16)) / (16 * 16);
				}

				index -= 2;
			}
			else if (event >= 0xD0 && event <= 0xDF)
			{
				eventChFix(i, event, 0xD0, index, instruments[i - 1]);
				index++;
			}
			else if (event == 0xFF) // メタイベント
			{
				int type = static_cast<int>(data[index++]);

				int dataSize = getDeltaTime(data, index);

				if (type == 0x2F) // トラック終端
				{
					break;
				}

				unsigned char* info = new unsigned char[dataSize];
				for (int j = 0; j < dataSize; ++j)
				{
					info[j] = data[index++];
				}

				if (type == 0x03) // トラック名
				{
					//_tracks[i].trackName = info;
				}
			}
			else if (i == 0 && event == 0xF0)
			{
				if (static_cast<int>(data[index]) == 0x05 && static_cast<int>(data[index + 1]) == 0x7E && static_cast<int>(data[index + 2]) == 0x7F && static_cast<int>(data[index + 3]) == 0x09 &&
					static_cast<int>(data[index + 4]) == 0x01 && static_cast<int>(data[index + 5]) == 0xF7) // GM Reset
				{
					_hasGmResetSysEx = true;
					index += 6;
				}
			}
			else // その他イベント
			{
				throw runtime_error(" unknown event found!");
				return 1;
			}
		}
	}

	// GM Rest
	if (!_hasGmResetSysEx)
	{
		unsigned char gmResetSysEx[] = { static_cast<unsigned char>(0x00), static_cast<unsigned char>(0xF0), static_cast<unsigned char>(0x05),
										static_cast<unsigned char>(0x7E), static_cast<unsigned char>(0x7F), static_cast<unsigned char>(0x09),
										static_cast<unsigned char>(0x01), static_cast<unsigned char>(0xF7) };

		size_t size = sizeof(gmResetSysEx);
		for (int j = 0; j < size; ++j)
		{
			_tracks[0].data.insert(_tracks[0].data.begin(), gmResetSysEx[size - 1 - j]);
		}
	}

	// 楽器情報の追加
	for (int i = 1; i < carrtoi(_header.trackSize, 2); ++i)
	{
		unsigned char programChange[] = { static_cast<unsigned char>(0x00), static_cast<unsigned char>(0xC0),instruments[i - 1] };
		if (instruments[i - 1] >= 128)
		{
			programChange[2] = static_cast<unsigned char>(0);
			programChange[1] = static_cast<unsigned char>(0xC9);
		}
		else
		{
			int ch = i - 1;
			if (i >= 9)
			{
				++ch;
			}

			ch %= 16;

			programChange[1] = static_cast<unsigned char>(0xC0 + ch);
		}
		_tracks[i].data.insert(_tracks[i].data.begin(), programChange[2]);
		_tracks[i].data.insert(_tracks[i].data.begin(), programChange[1]);
		_tracks[i].data.insert(_tracks[i].data.begin(), programChange[0]);
	}

	// 書き出し
	ofstream ofs(filePath, ios::out | ios::binary);

	ofs.write(reinterpret_cast<char*>(_header.MThd), sizeof(_header.MThd));
	ofs.write(reinterpret_cast<char*>(_header.dataSize), sizeof(_header.dataSize));
	ofs.write(reinterpret_cast<char*>(_header.format), sizeof(_header.format));
	ofs.write(reinterpret_cast<char*>(_header.trackSize), sizeof(_header.trackSize));
	ofs.write(reinterpret_cast<char*>(_header.timeUnit), sizeof(_header.timeUnit));

	for (int i = 0; i < carrtoi(_header.trackSize, 2); ++i)
	{
		ofs.write(reinterpret_cast<char*>(_tracks[i].MTrk), sizeof(_tracks[i].MTrk));

		int dataSize = carrtoi(_tracks[i].dataSize, 4);
		if (i != 0)
		{
			dataSize += 3;
		}
		else if (!_hasGmResetSysEx)
		{
			dataSize += 8;
		}

		int n = dataSize;
		for (int j = 0; j < 4; ++j)
		{
			_tracks[i].dataSize[3 - j] = n % (16 * 16);
			n = (n - n % (16 * 16)) / (16 * 16);
		}

		ofs.write(reinterpret_cast<char*>(_tracks[i].dataSize), sizeof(_tracks[i].dataSize));
		ofs.write(reinterpret_cast<char*>(_tracks[i].data.data()), _tracks[i].data.size());
	}

	return 0;
}

int MidiProcessor::carrtoi(unsigned char arr[], int size)
{
	int sum = 0;
	int x = 0;
	for (int i = 0; i < size; ++i, x += 2)
	{
		int second = static_cast<int>(arr[size - 1 - i]) / 16;
		int first = static_cast<int>(arr[size - 1 - i]) - 16 * second;
		sum += first * pow(16, x) + second * pow(16, x + 1);
	}

	return sum;
}

int MidiProcessor::getDeltaTime(unsigned char data[], int& index)
{
	vector<unsigned char> deltaTimeBits;
	unsigned char timeByte;
	int binary[8] = {};

	do {
		timeByte = data[index++];
		int n = static_cast<int>(timeByte);
		for (int i = 0; i < 8; ++i)
		{
			binary[i] = n % 2;
			n = (n - n % 2) / 2;
		}

		for (int i = 0; i < 7; ++i)
		{
			deltaTimeBits.emplace_back(binary[6 - i]);
		}
	} while (binary[7] == 1);

	int deltaTime = 0;
	for (int i = 0; i < deltaTimeBits.size(); ++i)
	{
		deltaTime += deltaTimeBits[i] * pow(2, deltaTimeBits.size() - 1 - i);
	}

	return deltaTime;
}

void MidiProcessor::eventChFix(int roopCounter, int event, int eventNumOfChOne, int index, unsigned char instrument)
{
	int correctNum = static_cast<int>(instrument) < 128 ? eventNumOfChOne + roopCounter - 1 : eventNumOfChOne + 9;

	if (event != correctNum)
	{
		int start = index - 1;
		_tracks[roopCounter].data.erase(_tracks[roopCounter].data.begin() + start, _tracks[roopCounter].data.begin() + start + 1);
		_tracks[roopCounter].data.insert(_tracks[roopCounter].data.begin() + start, static_cast<unsigned char>(correctNum));
	}
}

