using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using MidiWrapperNS;

namespace GmGen
{
    public partial class Form1 : Form
    {
        private MidiProcessorWrapper wrapper;

        public Form1()
        {
            InitializeComponent();
            wrapper = new MidiProcessorWrapper();

            InitializeTrackGrid();
        }

        private void InitializeTrackGrid()
        {
            dataGridViewTracks.AutoGenerateColumns = false;
            dataGridViewTracks.CurrentCellDirtyStateChanged += DataGridView1_CurrentCellDirtyStateChanged;
            dataGridViewTracks.Columns.Clear();

            // トラック名列
            DataGridViewTextBoxColumn trackNameCol = new DataGridViewTextBoxColumn();
            trackNameCol.HeaderText = "Track Name";
            trackNameCol.Name = "TrackName";
            trackNameCol.ReadOnly = true;
            trackNameCol.Width = 300;
            trackNameCol.SortMode = DataGridViewColumnSortMode.NotSortable;

            // 楽器列（ComboBox）
            DataGridViewComboBoxColumn instrumentCol = new DataGridViewComboBoxColumn();
            instrumentCol.HeaderText = "Instrument";
            instrumentCol.Name = "Instrument";
            instrumentCol.DropDownWidth = 200;
            instrumentCol.Width = 300;
            instrumentCol.Items.AddRange(gmInstruments);

            dataGridViewTracks.Columns.Add(trackNameCol);
            dataGridViewTracks.Columns.Add(instrumentCol);
        }


        private void btnOpen_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog dlg = new OpenFileDialog())
            {
                dlg.Filter = "MIDI Files|*.mid";
                if (dlg.ShowDialog() == DialogResult.OK)
                {
                    wrapper.ImportFile(dlg.FileName);

                    var tracks = wrapper.GetTrackNames();

                    dataGridViewTracks.Rows.Clear();

                    for (int i = 0; i < tracks.Length; ++i)
                    {
                        int index = dataGridViewTracks.Rows.Add();
                        dataGridViewTracks.Rows[index].Cells["TrackName"].Value = tracks[i];
                        dataGridViewTracks.Rows[index].Cells["Instrument"].Value = gmInstruments[wrapper.GetInitialInstrumentNum(i)]; // デフォルト
                    }
                }
            }
        }

        private void btnExport_Click(object sender, EventArgs e)
        {
            using (SaveFileDialog dlg = new SaveFileDialog())
            {
                dlg.Filter = "MIDI Files|*.mid";
                dlg.FileName = "export.mid";

                if (dlg.ShowDialog() == DialogResult.OK)
                {
                    int rowCount = dataGridViewTracks.Rows.Count;
                    byte[] instruments = new byte[rowCount];

                    for (int i = 0; i < rowCount; i++)
                    {
                        var cellValue = dataGridViewTracks.Rows[i].Cells["Instrument"].Value as string;
                        if (!string.IsNullOrEmpty(cellValue))
                        {
                            // 先頭の「番号: 名前」形式から番号を取得
                            int colonIndex = cellValue.IndexOf(':');
                            if (colonIndex > 0)
                            {
                                string numberPart = cellValue.Substring(0, colonIndex).Trim();
                                if (byte.TryParse(numberPart, out byte program))
                                {
                                    instruments[i] = program;
                                }
                            }
                        }
                    }

                    try
                    {
                        wrapper.ExportFile(dlg.FileName, instruments);
                        MessageBox.Show("Export Completed", "Success", MessageBoxButtons.OK, MessageBoxIcon.Information);
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show("An error occur in expoting:\n" + ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                }
            }
        }

        private void DataGridView1_CurrentCellDirtyStateChanged(object sender, EventArgs e)
        {
            var dgv = sender as DataGridView;

            // 編集中のセルがあり、かつ変更が未確定なら
            if (dgv.IsCurrentCellDirty)
            {
                // 編集内容を確定させる
                dgv.CommitEdit(DataGridViewDataErrorContexts.Commit);
            }
        }


        private string[] gmInstruments = new string[]
        {
            "0: Acoustic Grand Piano",
            "1: Bright Acoustic Piano",
            "2: Electric Grand Piano",
            "3: Honky-tonk Piano",
            "4: Electric Piano 1",
            "5: Electric Piano 2",
            "6: Harpsichord",
            "7: Clavinet",
            "8: Celesta",
            "9: Glockenspiel",
            "10: Music Box",
            "11: Vibraphone",
            "12: Marimba",
            "13: Xylophone",
            "14: Tubular Bells",
            "15: Dulcimer",
            "16: Drawbar Organ",
            "17: Percussive Organ",
            "18: Rock Organ",
            "19: Church Organ",
            "20: Reed Organ",
            "21: Accordion",
            "22: Harmonica",
            "23: Tango Accordion",
            "24: Acoustic Guitar (nylon)",
            "25: Acoustic Guitar (steel)",
            "26: Electric Guitar (jazz)",
            "27: Electric Guitar (clean)",
            "28: Electric Guitar (muted)",
            "29: Overdriven Guitar",
            "30: Distortion Guitar",
            "31: Guitar harmonics",
            "32: Acoustic Bass",
            "33: Electric Bass (finger)",
            "34: Electric Bass (pick)",
            "35: Fretless Bass",
            "36: Slap Bass 1",
            "37: Slap Bass 2",
            "38: Synth Bass 1",
            "39: Synth Bass 2",
            "40: Violin",
            "41: Viola",
            "42: Cello",
            "43: Contrabass",
            "44: Tremolo Strings",
            "45: Pizzicato Strings",
            "46: Orchestral Harp",
            "47: Timpani",
            "48: String Ensemble 1",
            "49: String Ensemble 2",
            "50: SynthStrings 1",
            "51: SynthStrings 2",
            "52: Choir Aahs",
            "53: Voice Oohs",
            "54: Synth Voice",
            "55: Orchestra Hit",
            "56: Trumpet",
            "57: Trombone",
            "58: Tuba",
            "59: Muted Trumpet",
            "60: French Horn",
            "61: Brass Section",
            "62: SynthBrass 1",
            "63: SynthBrass 2",
            "64: Soprano Sax",
            "65: Alto Sax",
            "66: Tenor Sax",
            "67: Baritone Sax",
            "68: Oboe",
            "69: English Horn",
            "70: Bassoon",
            "71: Clarinet",
            "72: Piccolo",
            "73: Flute",
            "74: Recorder",
            "75: Pan Flute",
            "76: Blown Bottle",
            "77: Shakuhachi",
            "78: Whistle",
            "79: Ocarina",
            "80: Lead 1 (square)",
            "81: Lead 2 (sawtooth)",
            "82: Lead 3 (calliope)",
            "83: Lead 4 (chiff)",
            "84: Lead 5 (charang)",
            "85: Lead 6 (voice)",
            "86: Lead 7 (fifths)",
            "87: Lead 8 (bass+lead)",
            "88: Pad 1 (new age)",
            "89: Pad 2 (warm)",
            "90: Pad 3 (polysynth)",
            "91: Pad 4 (choir)",
            "92: Pad 5 (bowed)",
            "93: Pad 6 (metallic)",
            "94: Pad 7 (halo)",
            "95: Pad 8 (sweep)",
            "96: FX 1 (rain)",
            "97: FX 2 (soundtrack)",
            "98: FX 3 (crystal)",
            "99: FX 4 (atmosphere)",
            "100: FX 5 (brightness)",
            "101: FX 6 (goblins)",
            "102: FX 7 (echoes)",
            "103: FX 8 (sci-fi)",
            "104: Sitar",
            "105: Banjo",
            "106: Shamisen",
            "107: Koto",
            "108: Kalimba",
            "109: Bagpipe",
            "110: Fiddle",
            "111: Shanai",
            "112: Tinkle Bell",
            "113: Agogo",
            "114: Steel Drums",
            "115: Woodblock",
            "116: Taiko Drum",
            "117: Melodic Tom",
            "118: Synth Drum",
            "119: Reverse Cymbal",
            "120: Guitar Fret Noise",
            "121: Breath Noise",
            "122: Seashore",
            "123: Bird Tweet",
            "124: Telephone Ring",
            "125: Helicopter",
            "126: Applause",
            "127: Gunshot",
            "128: Drums",
        };

        private void dataGridViewTracks_CellContentClick(object sender, DataGridViewCellEventArgs e)
        {

        }
    }
}
