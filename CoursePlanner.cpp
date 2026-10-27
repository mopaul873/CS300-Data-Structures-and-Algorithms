//============================================================================
// Name        : ProjectTwo.cpp
// Author      : Paul Momo
// Course      : CS 300 - Project Two
// Description : Advising program for ABCU. Loads the course file into a
//               binary search tree, prints a sorted list of all courses,
//               and prints the title and prerequisites for one course.
//============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>

using namespace std;

// holds the information for one course
struct Course {
    string courseNumber;
    string courseName;
    vector<string> prerequisites;
};

// one node in the binary search tree
struct Node {
    Course course;
    Node* left;
    Node* right;

    Node(Course aCourse) {
        course = aCourse;
        left = nullptr;
        right = nullptr;
    }
};

// removes spaces, tabs and line endings from the start and end of a string
string trim(string text) {
    size_t start = text.find_first_not_of(" \t\r\n");
    if (start == string::npos) {
        return "";
    }
    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

// changes a string to all uppercase so csci400 and CSCI400 match
string toUpper(string text) {
    for (size_t i = 0; i < text.length(); i++) {
        text[i] = toupper(text[i]);
    }
    return text;
}

//============================================================================
// Binary search tree class
// I picked the tree in Project One because it keeps the courses in order,
// so the sorted list comes from an in-order traversal with no extra sorting.
//============================================================================
class CourseTree {
private:
    Node* root;

    void addNode(Node* node, Course course);
    void inOrder(Node* node);
    void destroyTree(Node* node);

public:
    CourseTree();
    ~CourseTree();
    void Insert(Course course);
    Course* Search(string courseNumber);
    void PrintCourseList();
    void Clear();
    bool IsEmpty();
};

// start with an empty tree
CourseTree::CourseTree() {
    root = nullptr;
}

// free all the nodes when the program ends
CourseTree::~CourseTree() {
    destroyTree(root);
}

// deletes every node, children first
void CourseTree::destroyTree(Node* node) {
    if (node != nullptr) {
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
}

// empties the tree so the file can be loaded again
void CourseTree::Clear() {
    destroyTree(root);
    root = nullptr;
}

bool CourseTree::IsEmpty() {
    return root == nullptr;
}

// adds a course to the tree
void CourseTree::Insert(Course course) {
    if (root == nullptr) {
        root = new Node(course);
    }
    else {
        addNode(root, course);
    }
}

// finds the right spot for the new course (smaller goes left, bigger goes right)
void CourseTree::addNode(Node* node, Course course) {
    if (course.courseNumber < node->course.courseNumber) {
        if (node->left == nullptr) {
            node->left = new Node(course);
        }
        else {
            addNode(node->left, course);
        }
    }
    else {
        if (node->right == nullptr) {
            node->right = new Node(course);
        }
        else {
            addNode(node->right, course);
        }
    }
}

// looks for a course by number, returns nullptr if it is not in the tree
Course* CourseTree::Search(string courseNumber) {
    Node* current = root;

    while (current != nullptr) {
        if (courseNumber == current->course.courseNumber) {
            return &current->course;
        }
        else if (courseNumber < current->course.courseNumber) {
            current = current->left;
        }
        else {
            current = current->right;
        }
    }
    return nullptr;
}

// prints every course in alphanumeric order
void CourseTree::PrintCourseList() {
    inOrder(root);
}

// in-order traversal: left, current, right
void CourseTree::inOrder(Node* node) {
    if (node != nullptr) {
        inOrder(node->left);
        cout << node->course.courseNumber << ", " << node->course.courseName << endl;
        inOrder(node->right);
    }
}

//============================================================================
// Reads the course file and loads it into the tree
// Returns true if at least one course was loaded
//============================================================================
bool loadCourses(string fileName, CourseTree& tree) {
    ifstream file(fileName);

    if (!file.is_open()) {
        cout << "Error: could not open the file " << fileName << endl;
        return false;
    }

    vector<Course> courses;   // holds the courses until the prerequisites are checked
    string line;
    int lineNumber = 0;

    while (getline(file, line)) {
        lineNumber++;

        // Excel sometimes adds hidden characters at the start of the file
        if (lineNumber == 1 && line.size() >= 3 && line.substr(0, 3) == "\xEF\xBB\xBF") {
            line = line.substr(3);
        }

        // skip blank lines
        if (trim(line) == "") {
            continue;
        }

        // split the line at each comma
        vector<string> tokens;
        stringstream ss(line);
        string token;
        while (getline(ss, token, ',')) {
            token = trim(token);
            if (token != "") {      // ignore empty fields like trailing commas
                tokens.push_back(token);
            }
        }

        // every line needs at least a course number and a name
        if (tokens.size() < 2) {
            cout << "Format error on line " << lineNumber
                 << ": a course needs a number and a name. Line skipped." << endl;
            continue;
        }

        Course course;
        course.courseNumber = toUpper(tokens[0]);
        course.courseName = tokens[1];
        for (size_t i = 2; i < tokens.size(); i++) {
            course.prerequisites.push_back(toUpper(tokens[i]));
        }
        courses.push_back(course);
    }
    file.close();

    if (courses.empty()) {
        cout << "Error: no courses were found in " << fileName << endl;
        return false;
    }

    // clear out old data in case the file is loaded a second time
    tree.Clear();

    // check that each prerequisite is a course in the file, then add it to the tree
    for (size_t i = 0; i < courses.size(); i++) {
        for (size_t j = 0; j < courses[i].prerequisites.size(); j++) {
            bool found = false;
            for (size_t k = 0; k < courses.size(); k++) {
                if (courses[k].courseNumber == courses[i].prerequisites[j]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                cout << "Warning: prerequisite " << courses[i].prerequisites[j]
                     << " for " << courses[i].courseNumber
                     << " is not a course in the file." << endl;
            }
        }

        // don't add the same course twice
        if (tree.Search(courses[i].courseNumber) != nullptr) {
            cout << "Warning: " << courses[i].courseNumber
                 << " is listed more than once. Only the first one was kept." << endl;
        }
        else {
            tree.Insert(courses[i]);
        }
    }

    cout << "Courses loaded successfully." << endl;
    return true;
}

// prints the title and prerequisites for one course
void printCourse(CourseTree& tree, string courseNumber) {
    Course* course = tree.Search(courseNumber);

    if (course == nullptr) {
        cout << courseNumber << " was not found." << endl;
        return;
    }

    cout << course->courseNumber << ", " << course->courseName << endl;

    if (course->prerequisites.empty()) {
        cout << "Prerequisites: None" << endl;
        return;
    }

    // first line matches the sample output, then each prerequisite with its title
    cout << "Prerequisites: ";
    for (size_t i = 0; i < course->prerequisites.size(); i++) {
        cout << course->prerequisites[i];
        if (i < course->prerequisites.size() - 1) {
            cout << ", ";
        }
    }
    cout << endl;

    for (size_t i = 0; i < course->prerequisites.size(); i++) {
        Course* prereq = tree.Search(course->prerequisites[i]);
        if (prereq != nullptr) {
            cout << "    " << prereq->courseNumber << ", " << prereq->courseName << endl;
        }
        else {
            cout << "    " << course->prerequisites[i] << " (not found in the course list)" << endl;
        }
    }
}

// shows the menu options
void displayMenu() {
    cout << endl;
    cout << "  1. Load Data Structure." << endl;
    cout << "  2. Print Course List." << endl;
    cout << "  3. Print Course." << endl;
    cout << "  9. Exit" << endl;
    cout << endl;
    cout << "What would you like to do? ";
}

int main() {
    CourseTree courseTree;
    bool dataLoaded = false;
    string choice = "";

    cout << "Welcome to the course planner." << endl;

    while (choice != "9") {
        displayMenu();

        // read the whole line as text so letters or extra input don't break the program
        if (!getline(cin, choice)) {
            break;   // input ended
        }
        choice = trim(choice);

        if (choice == "1") {
            string fileName;
            cout << "Enter the name of the course file: ";
            getline(cin, fileName);
            fileName = trim(fileName);

            // keep the old data if the new file fails to load
            if (loadCourses(fileName, courseTree)) {
                dataLoaded = true;
            }
        }
        else if (choice == "2") {
            if (!dataLoaded) {
                cout << "Please load the data first (option 1)." << endl;
            }
            else {
                cout << "Here is a sample schedule:" << endl;
                cout << endl;
                courseTree.PrintCourseList();
            }
        }
        else if (choice == "3") {
            if (!dataLoaded) {
                cout << "Please load the data first (option 1)." << endl;
            }
            else {
                string courseNumber;
                cout << "What course do you want to know about? ";
                getline(cin, courseNumber);
                courseNumber = toUpper(trim(courseNumber));

                if (courseNumber == "") {
                    cout << "Error: no course number was entered." << endl;
                }
                else {
                    printCourse(courseTree, courseNumber);
                }
            }
        }
        else if (choice == "9") {
            cout << "Thank you for using the course planner!" << endl;
        }
        else {
            cout << choice << " is not a valid option." << endl;
        }
    }

    return 0;
}
