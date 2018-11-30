//
// Created by bilin on 11/29/18.
//

#include "../include/data_handler.h"
#include <vector>
#include <algorithm>

using namespace std;

vector<int> data_socket_list;

bool isData(int sock_index) {
    vector<int>::iterator it = find(data_socket_list.begin(), data_socket_list.end(), sock_index);
    return it != data_socket_list.end();
}