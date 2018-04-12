using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Text.RegularExpressions;
using SS;
using SpreadsheetUtilities;
using NetworkController;
using System.Net.Sockets;

namespace SpreadsheetGUI
{
    /// <summary>
    /// A class to handle user control and to display the view
    /// </summary>
    public partial class Form1 : Form
    {
        private Socket theServer;

        // A sheet to store the backgroud logic.
        private Spreadsheet underLyingSheet;
        

        /// <summary>
        /// constructor use to initialize the GUI
        /// </summary>
        public Form1()
        {
            InitializeComponent();

            // initialize the sheet
            underLyingSheet = new Spreadsheet
                                (s => Regex.IsMatch(s, @"^[a-zA-Z][1-9][0-9]?$"), s => s.ToUpper(), "ps6");

            
            // handler used to update current selected cell information
            spreadsheetPanel1.SelectionChanged += updateCellMessage;
            foreach (Control control in this.Controls)
            {
                control.PreviewKeyDown += new PreviewKeyDownEventHandler(control_PreviewKeyDown);
            }
            textBox1.Text = "A1";
        }
        void control_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            if (e.KeyCode == Keys.Up || e.KeyCode == Keys.Down || e.KeyCode == Keys.Left || e.KeyCode == Keys.Right)
            {
                e.IsInputKey = true;
                int col;
                int row;
                spreadsheetPanel1.GetSelection(out col, out row);

                switch (e.KeyCode)
                {
                    case Keys.Up:
                        spreadsheetPanel1.SetSelection(col, row - 1);
                        break;
                    case Keys.Down:
                        spreadsheetPanel1.SetSelection(col, row + 1);
                        break;
                    case Keys.Left:
                        spreadsheetPanel1.SetSelection(col - 1, row);
                        break;
                    case Keys.Right:
                        spreadsheetPanel1.SetSelection(col + 1, row);
                        break;

                }

                updateCellMessage(spreadsheetPanel1);

            }
        }
        /// <summary>
        /// Method used to detect an "Click" input, 
        /// if an "Click" is catched on "open strip", 
        /// start a new window. 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Form1 newForm = new Form1();
            newForm.Show();
        }

        /// <summary>
        /// Method used to detect an "Click" input, 
        /// if an "Click" is catched on "save strip", 
        /// determine wether to save and perform saving. 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void loadToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (textBox3.Text == "")
            {
                MessageBox.Show("Please enter a server address");
                return;
            }

            if (textBox3.Text == "")
            {
                MessageBox.Show("Please enter a player name");
                return;
            }

            // try read the target sheet, and save it.
            try
            {
                theServer = Networking.ConnectToServer(FirstContact, textBox5.Text);

            }

            // display read error message
            catch (SpreadsheetReadWriteException)
            {
                MessageBox.Show("Please enter a valid server address");
            }
        }

        /// <summary>
        /// The method used for the first contact from client to server
        /// </summary>
        /// <param name="state"></param>
        private void FirstContact(SocketState state)
        {
            // If this was the SpaceWars game, we would need to start the "handshake" (send player name)
            // send name
            byte[] messageBytes = Encoding.UTF8.GetBytes("register" + (char)3);
            theServer.Send(messageBytes, messageBytes.Length, SocketFlags.None);

            // Change the action that is take when a network event occurs. Now when data is received,
            // the Networking library will invoke ProcessMessage (see below)

            state.callMe = ReceiveStartup;
            // If this was the SpaceWars game, there would be one more step of the handshake, and we wouldn't
            // go straight in to ProcessMessage

            Networking.GetData(state);
        }
        private void Load(SocketState state)
        {

        }
        /// <summary>
        /// If this was a SpaceWars game, this method would handle the final step of the handshake.
        /// </summary>
        /// <param name="state"></param>
        private void ReceiveStartup(SocketState state)
        {
            
            string totalData = state.sb.ToString();
            textBox3.Text = totalData;
            state.callMe = ProcessingData;
            // Start waiting for data
            Networking.GetData(state);
        }

        private void ProcessingData(SocketState state)
        {

            string totalData = state.sb.ToString();
            textBox3.Text = totalData;
            // Start waiting for data
            Networking.GetData(state);
        }

        /// <summary>
        /// Method used to detect an "Click" input, 
        /// if an "Click" is catched on "close strip", 
        /// first determine wether to save, 
        /// then close the form.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void closeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }

        /// <summary>
        /// Override the OnFormClosing such that when clicking the close button
        /// in the corner, it will ask to save.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            // if no need to save, close it 
            if ((underLyingSheet.Changed))
            {
                // get the clicked button
            }
        }

        /// <summary>
        /// Method used to detect an "enter" input, 
        /// if an "enter" is catched, set content and do evaluation of the selected cell.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void textBox3_KeyDown(object sender, KeyEventArgs e)
        {

            if (e.KeyCode == Keys.Enter)
            {
                String cellName = GetNameOnSelect();
                String cellContent = textBox3.Text;
                byte[] messageBytes = Encoding.UTF8.GetBytes("edit"+" "+cellName+":"+cellContent);
                // set content and do evaluation of the selected cell 
                try
                {
                    Networking.Send(theServer, "edit" + " " + cellName + ":" + cellContent);
                }
                // something shouldn't happen
                catch (Exception)
                {
                    textBox3.Text = "Error: unexpected error";
                }
            }
        }

        /// <summary>
        /// A helper method used to update the information 
        /// in the top three text boxs.
        /// </summary>
        /// <param name="ss"></param>
        private void updateCellMessage(SpreadsheetPanel ss)
        {
            // let the left box display the cell name
            String cellName = GetNameOnSelect();
            textBox1.Text = cellName;

            // let the right box display the content
            Object cellContent = underLyingSheet.GetCellContents(cellName);
            if (cellContent is Formula)
            {
                textBox3.Text = "=" + cellContent.ToString();
            }
            else
            {
                textBox3.Text = cellContent.ToString();
            }

            // let the right box display the value
            Object cellValue = underLyingSheet.GetCellValue(cellName);
            textBox2.Text = cellValue.ToString();

        }

        /// <summary>
        /// A helper method used to get the cell name of the current selection
        /// on the spreadsheet.
        /// </summary>
        /// <returns></returns>
        private string GetNameOnSelect()
        {
            int col;
            int row;    
            
            // Get the location of the current selection
            spreadsheetPanel1.GetSelection(out col, out row);

            // Covert the location to a cell name
            row++;
            char Col = (char)('A' + col);

            return "" + Col + row;
        }

        /// <summary>
        /// Method to display help information.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void helpToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MessageBox.Show("Use mouse for selecting cells. \n" +
                            "Use the most right textbox for editing contents of cells. \n" +
                            "Pressing Enter button for entering the new contents. \n"+ 
                            "Clicking on any of the functional strip will take the value of the current selected cell value as input and compute the result using selected method.\n"+
                            "The result will be saved back to the cell as value and content.");
        }



        private void spreadsheetPanel1_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                TextBox tbx = this.Controls.Find("txt", true).FirstOrDefault() as TextBox;
                if (tbx != null)
                {
                    spreadsheetPanel1.Focus();
                    Controls.Remove(tbx);
                    //Networking.Send(theServer, “unfocus"+" "+"cellname"+"\3”);
                }
                else
                {
                    int col;
                    int row;
                    spreadsheetPanel1.GetSelection(out col, out row);
                    string cellname = GetNameOnSelect();
                    TextBox txt = new TextBox();
                    txt.Name = "txt";
                    txt.Text = textBox3.Text;
                    txt.Width = 80;
                    txt.Height = 20;
                    txt.KeyDown += new KeyEventHandler(txtKeyDown);
                    txt.Location = new Point(31+col * 80, 81+row * 20);
                    this.Controls.Add(txt);
                    txt.BringToFront();
                    txt.Focus();
                    //Networking.Send(theServer, “focus"+" "+"cellname"+"\3”);
                }
            }
        }

        private void txtKeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                TextBox tbx = this.Controls.Find("txt", true).FirstOrDefault() as TextBox;

                String cellName = GetNameOnSelect();
                String cellContent = tbx.Text;

                // set content and do evaluation of the selected cell
                try
                {
                    underLyingSheet.SetContentsOfCell(cellName, cellContent);
                    updateCellMessage(spreadsheetPanel1);
                    int col;
                    int row;
                    spreadsheetPanel1.GetSelection(out col, out row);
                    spreadsheetPanel1.SetValue(col, row, tbx.Text);
                }

                // display circular dependecy error message
                catch (CircularException)
                {
                    textBox3.Text = "Error: circular dependecy";
                }

                // display invalid formula error message
                catch (FormulaFormatException)
                {
                    textBox3.Text = "Error: invalid formula format";
                }

                // something shouldn't happen
                catch (Exception)
                {
                    textBox3.Text = "Error: unexpected error";
                }

                //Networking.Send(theServer, "edit" + " " + cellName + ":" + cellContent);
                spreadsheetPanel1.Focus();
                Controls.Remove(tbx);
            }
        }

        private void textBox5_TextChanged(object sender, EventArgs e)
        {

        }
    }
}
