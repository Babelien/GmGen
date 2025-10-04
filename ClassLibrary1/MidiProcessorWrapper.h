#pragma once
#include "MidiProcessor.h"
#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace MidiWrapperNS {
    public ref class MidiProcessorWrapper
    {
    private:
        MidiProcessor* proc;
    public:
        MidiProcessorWrapper() { proc = new MidiProcessor(); }

        ~MidiProcessorWrapper() { delete proc; }

        void ImportFile(System::String^ path) {
            IntPtr p = Marshal::StringToHGlobalAnsi(path);
            proc->importMidi(static_cast<char*>(p.ToPointer()));
            Marshal::FreeHGlobal(p);
        }

        void ExportFile(System::String^ path, cli::array<System::Byte>^ instruments)
        {
            // .NET �� String^ �� const char* �֕ϊ��iANSI�j
            IntPtr p = Marshal::StringToHGlobalAnsi(path);

            // .NET �� byte[] ���l�C�e�B�u unsigned char* �֌Œ�
            pin_ptr<unsigned char> pInstruments = &instruments[0];

            // �l�C�e�B�u�֐��Ăяo��
            proc->exportMidi(static_cast<const char*>(p.ToPointer()), pInstruments);

            // �m�ۂ��������������
            Marshal::FreeHGlobal(p);
        }

        array<String^>^ GetTrackNames() {
            auto names = proc->getTrackNames(); // vector<string>
            array<String^>^ result = gcnew array<String^>(names.size());
            for (int i = 0; i < names.size(); i++)
                result[i] = gcnew String(names[i].c_str());
            return result;
        }

        int GetInitialInstrumentNum(int trackNum)
        {
            return static_cast<int>(proc->getTracks()[trackNum+1].instrument);
        }
    };
}
