/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

#pragma once

#include "json.h"
#include "transport_catalogue.h"

#include <iostream>

class JsonReader {
public:
    JsonReader(std::istream& input)
        : input_(json::Load(input)) {}

    const json::Node& ReadBaseRequests() const;
    const json::Node& ReadStatRequests() const;

    void FillCatalogue(Transport::Catalogue& catalogue);

private:
    json::Document input_;
    json::Node null_ = nullptr;
};