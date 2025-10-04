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
            // .NET の String^ → const char* へ変換（ANSI）
            IntPtr p = Marshal::StringToHGlobalAnsi(path);

            // .NET の byte[] をネイティブ unsigned char* へ固定
            pin_ptr<unsigned char> pInstruments = &instruments[0];

            // ネイティブ関数呼び出し
            proc->exportMidi(static_cast<const char*>(p.ToPointer()), pInstruments);

            // 確保したメモリを解放
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
