#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    Transport::Catalogue catalogue;

    InputData::ProcessCommands(cin, catalogue);

    ProcessRequestsInfo(cin, catalogue, cout);
}