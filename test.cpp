#include <SFML/Window.hpp>
#include <unordered_set>
#include <iostream>

int main()
{
    std::unordered_multiset<int> positions;
    positions.insert(1);
    positions.insert(2);
    positions.insert(3);
    positions.insert(4);
    positions.insert(5);
    positions.insert(6);
    positions.insert(7);
    positions.insert(8);
    positions.insert(9);
    positions.insert(10);

}