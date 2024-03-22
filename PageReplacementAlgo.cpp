#include <queue>
#include <unordered_set>
#include <list>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
using namespace std;

struct PageFrame {
    char page;
    bool referenced;
};

void pageFaultFIFO(const std::vector<char>& pages, int capacity) {
    std::queue<char> frameQueue;
    std::vector<char> frames(capacity, '-'); // Initialize frames with '-' indicating empty
    int pageFaults = 0;

    for (char page : pages) {
        std::cout << "Sequence Number: " << &page - &pages[0] << "\n";
        std::cout << "Reference: " << page << "\n";

        // Check if the page is already in the frame
        auto it = std::find(frames.begin(), frames.end(), page);
        bool isPageFault = (it == frames.end());

        // If it is a page fault and there is no room, remove the oldest page
        if (isPageFault && frameQueue.size() == capacity) {
            char oldPage = frameQueue.front();
            frameQueue.pop();
            *std::find(frames.begin(), frames.end(), oldPage) = page; // Replace the old page with the new page
        } else if (isPageFault) {
            // If it is a page fault and there is room, add the new page
            frames[frameQueue.size()] = page;
        }

        // Add the new page to the queue if it's a page fault
        if (isPageFault) {
            frameQueue.push(page);
            pageFaults++;
            std::cout << "Page Fault: true\n";
        } else {
            std::cout << "Page Fault: false\n";
        }

        // Display the frame status
        for (int i = 0; i < capacity; ++i) {
            std::cout << "Frame " << i + 1 << ": ";
            if (frames[i] != '-') {
                std::cout << frames[i];
            } else {
                std::cout << "null";
            }
            if (isPageFault && frameQueue.front() == frames[i]) {
                std::cout << " (victim)";
            }
            std::cout << "\n";
        }
        std::cout << "\n"; // Newline for formatting
    }

    std::cout << "Total page faults: " << pageFaults << "\n";
}

void pageFaultLRU(const std::vector<char>& pages, int capacity) {
    std::list<char> framesList; // Stores the pages in the order of usage
    std::unordered_map<char, std::list<char>::iterator> pagesMap; // Maps each page to its position in framesList
    int pageFaults = 0;

    for (size_t i = 0; i < pages.size(); ++i) {
        std::cout << "Sequence Number: " << i << "\n";
        std::cout << "Reference: " << pages[i] << "\n";

        auto it = pagesMap.find(pages[i]);

        // Page not in frames, we have a page fault
        if (it == pagesMap.end()) {
            // Check if we need to remove the least recently used page
            if (framesList.size() == capacity) {
                char last = framesList.back();
                framesList.pop_back(); // Remove the least recently used page
                pagesMap.erase(last); // Remove it from the map
            }
            pageFaults++;
            std::cout << "Page Fault: true\n";
        } else {
            // Page is in frames, move it to the front to mark it as recently used
            framesList.erase(it->second);
            std::cout << "Page Fault: false\n";
        }

        // Add new page to the front of the list and in the map
        framesList.push_front(pages[i]);
        pagesMap[pages[i]] = framesList.begin();

        // Display the frame status
        int j = 0;
        for (auto page : framesList) {
            std::cout << "Frame " << j + 1 << ": " << page << "\n";
            j++;
        }
        for (; j < capacity; j++) {
            std::cout << "Frame " << j + 1 << ": null\n";
        }
        std::cout << "\n"; // Newline for formatting
    }

    std::cout << "Total page faults: " << pageFaults << "\n";
}


void pageFaultClock(const std::vector<char>& pages, int capacity) {
    std::vector<PageFrame> frameVector(capacity, {'-', false});
    int pageFaults = 0;
    int hand = 0;  // The "hand" of the clock

    for (size_t i = 0; i < pages.size(); ++i) {
        std::cout << "Sequence Number: " << i << "\n";
        std::cout << "Reference: " << pages[i] << "\n";

        // Check if the page is in any frame
        bool pageFound = false;
        for (auto& frame : frameVector) {
            if (frame.page == pages[i]) {
                frame.referenced = true; // Mark as referenced
                pageFound = true;
                break;
            }
        }

        if (!pageFound) {
            while (frameVector[hand].referenced) {
                frameVector[hand].referenced = false; // Unset the reference bit
                hand = (hand + 1) % capacity; // Move the hand forward
            }
            
            // Replace the page at the hand
            frameVector[hand].page = pages[i];
            frameVector[hand].referenced = true; // Set the reference bit
            hand = (hand + 1) % capacity; // Move the hand forward

            pageFaults++;
            std::cout << "Page Fault: true\n";
        } else {
            std::cout << "Page Fault: false\n";
        }

        // Display the frame status
        for (int j = 0; j < capacity; ++j) {
            std::cout << "Frame " << j + 1 << ": ";
            if (frameVector[j].page != '-') {
                std::cout << frameVector[j].page << (frameVector[j].referenced ? "*" : "");
            } else {
                std::cout << "null";
            }
            std::cout << "\n";
        }
        std::cout << "\n"; // Newline for formatting
    }

    std::cout << "Total page faults: " << pageFaults << "\n";
}



int findOptimalVictim(const std::vector<char>& pages, std::vector<char>& frames, int currentPageIndex) {
    int index = -1;
    int farthest = currentPageIndex;
    for (int i = 0; i < frames.size(); ++i) {
        int j;
        for (j = currentPageIndex; j < pages.size(); ++j) {
            if (frames[i] == pages[j]) {
                if (j > farthest) {
                    farthest = j;
                    index = i;
                }
                break;
            }
        }
        if (j == pages.size()) {
            return i; // If the page isn't going to be used again, return this frame
        }
    }
    return (index == -1) ? 0 : index; // If all are going to be used, replace the last used
}

void pageFaultOptimal(const std::vector<char>& pages, int capacity) {
    std::vector<char> frames(capacity, '-'); // Initialize with '-' indicating empty
    std::unordered_map<char, bool> isInFrame;
    int pageFaults = 0;

    for (size_t i = 0; i < pages.size(); ++i) {
        std::cout << "Sequence Number: " << i << "\n";
        std::cout << "Reference: " << pages[i] << "\n";

        if (!isInFrame[pages[i]]) {
            int victimIndex = -1;
            if (isInFrame.size() >= capacity) {
                victimIndex = findOptimalVictim(pages, frames, i + 1);
                isInFrame.erase(frames[victimIndex]);
            }

            for (int j = 0; j < capacity; ++j) {
                if (frames[j] == '-' || j == victimIndex) {
                    frames[j] = pages[i];
                    isInFrame[pages[i]] = true;
                    pageFaults++;
                    break;
                }
            }
            std::cout << "Page Fault: true\n";
        } else {
            std::cout << "Page Fault: false\n";
        }

        // Display the frame status
        for (int j = 0; j < capacity; ++j) {
            std::cout << "Frame " << j + 1 << ": ";
            if (frames[j] != '-') {
                std::cout << frames[j];
            } else {
                std::cout << "null";
            }
            std::cout << "\n";
        }
        std::cout << "\n"; // Newline for formatting
    }

    std::cout << "Total page faults: " << pageFaults << "\n";
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cout << "Usage: " << argv[0] << " <algorithm choice> <number of frames> <page references...>" << endl;
        return 1;
    }

    int choice = atoi(argv[1]);
    int frames = atoi(argv[2]);
    if (frames <= 0 || choice < 1 || choice > 4) {
        cout << "Invalid input. Ensure correct algorithm choice and positive number of frames." << endl;
        return 1;
    }

    vector<char> pages;
    for (int i = 3; i < argc; i++) {
        pages.push_back(argv[i][0]); // Assuming single-character page references
    }

    switch (choice) {
        case 1:
            pageFaultFIFO(pages, frames);
            break;
        case 2:
            pageFaultLRU(pages, frames);
            break;
        case 3:
            pageFaultOptimal(pages, frames); // Corrected function name
            break;
        case 4:
            pageFaultClock(pages, frames);
            break;
        default:
            cout << "Invalid algorithm choice." << endl;
            return 1;
    }

    return 0;
}
