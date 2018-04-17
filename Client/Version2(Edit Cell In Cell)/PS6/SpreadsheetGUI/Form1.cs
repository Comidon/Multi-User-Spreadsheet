﻿using System;
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

namespace SpreadsheetGUI
{
    /// <summary>
    /// A class to handle user control and to display the view
    /// </summary>
    public partial class Form1 : Form
    {
        // A sheet to store the backgroud logic.
        private Spreadsheet underLyingSheet;

        // A bool to determine if a sheet have been saved before.
        private bool Saved;

        // A string use to store the filename (needed in save method)
        string FileName;

        /// <summary>
        /// constructor use to initialize the GUI
        /// </summary>
        public Form1()
        {
            InitializeComponent();

            // initialize the sheet
            underLyingSheet = new Spreadsheet
                                (s => Regex.IsMatch(s, @"^[a-zA-Z][1-9][0-9]?$"), s => s.ToUpper(), "ps6");

            // new sheet has never been saved
            Saved = false;
            FileName = "";

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
                        spreadsheetPanel1.SetSelection(col, row-1);
                        break;
                    case Keys.Down:
                        spreadsheetPanel1.SetSelection(col, row+1);
                        break;
                    case Keys.Left:
                        spreadsheetPanel1.SetSelection(col-1, row);
                        break;
                    case Keys.Right:
                        spreadsheetPanel1.SetSelection(col+1, row);
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
            // set filter
            openFileDialog1.Filter = "Spreadsheet Files (*.sprd)|*.sprd|" +
                                     "Text Files (*.txt)|*.txt|" +
                                     "XML Files (*.xml, *.xsl)|*.xml;*.xsl|" +
                                     "All Files (*.*)|*.*";

            // set settings up for open dialog
            openFileDialog1.DefaultExt = ".sprd";
            openFileDialog1.Title = "Load";
            openFileDialog1.ShowDialog();

            string LoadName = openFileDialog1.FileName;

            // before close the old sheet, ask to save
            if (underLyingSheet.Changed)
            {
                DialogResult result = MessageBox.Show("Would you like to save your changes?", "Confirm Close Current File",
                                                             MessageBoxButtons.YesNo, MessageBoxIcon.Question,
                                                             MessageBoxDefaultButton.Button1,
                                                             MessageBoxOptions.DefaultDesktopOnly);

                if (result == DialogResult.Yes)
                    saveToolStripMenuItem_Click(sender, e);
            }

            // try read the target sheet, and save it.
            try
            {
                // renew the sheet
                underLyingSheet = new Spreadsheet(LoadName, s => Regex.IsMatch(s, @"^[a-zA-Z][1-9][0-9]?$"),
                                                  s => s.ToUpper(), "ps6");

                // set currunt selection to default position
                spreadsheetPanel1.SetSelection(0, 0);
                updateCellMessage(spreadsheetPanel1);

                Saved = true;
                FileName = LoadName;
            }

            // display read error message
            catch (SpreadsheetReadWriteException)
            {
                MessageBox.Show("Error: Failed to read the selected file."
                    , "Loading Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
        
        /// <summary>
        /// Method used to detect an "Click" input, 
        /// if an "Click" is catched on "save strip", 
        /// determine wether to save and perform saving. 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // if the sheet haven't been saved to anywhere, preform saveas instead
            if (!Saved)
                saveAsToolStripMenuItem_Click(sender, e);

            // if the sheet already exists somewhere, resave it
            else
            {
                // save if have changes
                if (underLyingSheet.Changed)
                {
                    MessageBox.Show("Saved!");
                    underLyingSheet.Save(FileName);
                }

                // no save if no change
                else
                {
                    MessageBox.Show("No change need to be saved!");
                }
            }
        }

        /// <summary>
        /// Method used to detect an "Click" input, 
        /// if an "Click" is catched on "saveas strip", 
        /// perform save as. 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // setting filter for file types
            saveFileDialog1.Filter = "Spreadsheet Files (*.sprd)|*.sprd|" +
                                     "Text Files (*.txt)|*.txt|" +
                                     "XML Files (*.xml, *.xsl)|*.xml;*.xsl";

            // setting the settings for save dialog
            saveFileDialog1.DefaultExt = ".sprd";
            saveFileDialog1.Title = "SaveAs";
            saveFileDialog1.OverwritePrompt = true;
            saveFileDialog1.AddExtension = true;
            saveFileDialog1.ShowDialog();

            FileName = saveFileDialog1.FileName;

            // Put the save process in a try block to avoid ArgumentNullException
            try
            {
                underLyingSheet.Save(FileName);
            }

            // After catch an exception, do nothing since it's not saved
            catch (Exception)
            {
                // do nothing
            }

            // 
            if (FileName != "")
                Saved = true;
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
            /*// if no need to save, close it 
            if (!(underLyingSheet.Changed))
                Close();

            else
            {  
                // get the clicked button
                DialogResult confirmation = MessageBox.Show("Would you like to save your changes?", "Confirm Close",
                                                             MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question,
                                                             MessageBoxDefaultButton.Button1,
                                                             MessageBoxOptions.DefaultDesktopOnly);

                // choose to save
                if (confirmation == DialogResult.Yes)
                    saveToolStripMenuItem.PerformClick();

                // choose to cancel
                else if (confirmation == DialogResult.Cancel)
                    return;

                // (*choose to not save)
                Close();
            }*/
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
                DialogResult confirmation = MessageBox.Show("Would you like to save your changes?", "Confirm Close",
                                                             MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question,
                                                             MessageBoxDefaultButton.Button1,
                                                             MessageBoxOptions.DefaultDesktopOnly);

                // choose to save
                if (confirmation == DialogResult.Yes)
                    saveToolStripMenuItem.PerformClick();

                // choose to cancel
                else if (confirmation == DialogResult.Cancel)
                    e.Cancel = true;
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

                // set content and do evaluation of the selected cell
                try
                {
                    underLyingSheet.SetContentsOfCell(cellName, cellContent);
                    updateCellMessage(spreadsheetPanel1);
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

        /// <summary>
        /// A helper method that used to help do functional calculation.
        /// </summary>
        /// <param name="f"></param>
        private void functionLogic(Func<object, string> f)
        {

            object original = underLyingSheet.GetCellValue(GetNameOnSelect());

            // if value is double
            if (original is double)
            {
                // valid evaluation case
                if (!f(original).Equals("NaN"))
                {
                    underLyingSheet.SetContentsOfCell(GetNameOnSelect(), f(original));
                    updateCellMessage(spreadsheetPanel1);
                }

                // invalid evaluation case
                else
                {
                    textBox3.Text = "Can not use this function in this cell";
                }
            }

            // if value is string
            else
            {
                textBox3.Text = "Can not use this function in this cell";
            }
        }

        /// <summary>
        /// do square root
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void sqrtToolStripMenuItem_Click(object sender, EventArgs e)
        {
            functionLogic(x => Math.Sqrt((double)x).ToString());
        }

        /// <summary>
        /// do square
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void sqrToolStripMenuItem_Click(object sender, EventArgs e)
        {
            functionLogic(x => ((double)x * (double)x).ToString());
        }

        /// <summary>
        /// do cos
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cosToolStripMenuItem_Click(object sender, EventArgs e)
        {
            functionLogic(x => Math.Cos((double)x).ToString());
        }

        /// <summary>
        /// do sin
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void sinToolStripMenuItem_Click(object sender, EventArgs e)
        {
            functionLogic(x => Math.Sin((double)x).ToString());
        }

        /// <summary>
        /// do tan
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tanToolStripMenuItem_Click(object sender, EventArgs e)
        {
            functionLogic(x => Math.Tan((double)x).ToString());
        }

        /// <summary>
        /// do arccos
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void arcCosToolStripMenuItem_Click(object sender, EventArgs e)
        {
            functionLogic(x => Math.Acos((double)x).ToString());
        }

        /// <summary>
        /// do arcsin
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void arcSinToolStripMenuItem_Click(object sender, EventArgs e)
        {
            functionLogic(x => Math.Asin((double)x).ToString());
        }

        /// <summary>
        /// do arctan
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void arcTanToolStripMenuItem_Click(object sender, EventArgs e)
        {
            functionLogic(x => Math.Atan((double)x).ToString());
        }

        /// <summary>
        /// do Exp
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void expToolStripMenuItem_Click(object sender, EventArgs e)
        {
            functionLogic(x => Math.Exp((double)x).ToString());
        }

        /// <summary>
        /// do ln
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void logToolStripMenuItem_Click(object sender, EventArgs e)
        {
            functionLogic(x => Math.Log((double)x).ToString());
        }

        /// <summary>
        /// do log10
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void log10ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            functionLogic(x => Math.Log10((double)x).ToString());
        }

        /// <summary>
        /// do round
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void roundToolStripMenuItem_Click(object sender, EventArgs e)
        {
            functionLogic(x => Math.Round((double)x).ToString());
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


                spreadsheetPanel1.Focus();
                Controls.Remove(tbx);
            }
        }

    }
}
