#include <iostream>
#include <string>

#include "jsonValue.h"

int jsonPrintTest(std::string jsonSt)
{
    json_value_t *val = json_value_parse(jsonSt.c_str());

    if (val != NULL)
    {
        print_json_value(val, 0);
        json_value_destroy(val);
    }
    else
    {
        std::cout << "json_value_parse failed" << std::endl;
    }

    return 0;
}

int main(int argc, char const *argv[])
{
    std::string jsonSt = "{\"name\":\"Jack (\\\"Bee\\\") Nimble\", \"format\":{\"type\":\"rect\", \"width\":1920, \"height\":1080, \"interlace\":false, \"frame rate\":24}}";
    jsonPrintTest(jsonSt);

    return 0;
}
