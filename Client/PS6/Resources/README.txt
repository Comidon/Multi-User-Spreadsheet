Author : Yunxiao Cai, Dong Wang
Date : Oct / 27 / 2017


# External Code Resources

Skeleton copied from given example.
References from MSDN are used to form SaveAs, Open, Close methods.
Idea about overriding OnFormClosing(FormClosingEventArgs e) is inspired by StackOverflow.


# Basic Instruction

Click on cell to select.
Enter content to selected cell by typing into the top right text box.
After finish typing in the text box, press Enter to set content to cell and evalute the value.


# Additional Features:

We add a function menu which contains several math method including "sin" "cos" "tan" "log" "sqrt" and so on.
Clicking on any of the functional strip will take the value of the current selected cell value as input and compute
the result using selected method. The result will be saved back to the cell as value and content.


# Design Decisions

We overrided OnFormClosing(FormClosingEventArgs e) method. The reason we did this was that we wanted the close button
(the one at the top right) to be involved in data safety feature (ask for saving before closing).


# Versions of the PS2/PS3/PS4 libraries:

PS2 : v 1.0
PS3 : v 1.0
PS4 : v 1.0