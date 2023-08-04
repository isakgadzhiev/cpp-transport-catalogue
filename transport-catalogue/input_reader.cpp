#include "input_reader.h"

namespace input_reader {
    using namespace std;
    using namespace std::literals;

    namespace detail_input {
        string ReadLine(istream& input) {
            string s;
            getline(input, s);
            return s;
        }

        int ReadLineWithNumber(istream& input) {
            int count;
            input >> count;
            ReadLine(input);
            return count;
        }
    }

    void CutPartOfRequest(string_view& request, const char& delimiter) {
        size_t pos = request.find(delimiter);
        request.remove_prefix(pos + 2);
    }
    void CutPartOfRequest(string_view& request, const size_t& pos) {
        request.remove_prefix(pos + 2);
    }

    vector<string> SplitIntoStops(string_view text, Bus& bus) {
        vector<string> words;
        string word;
        char delimiter;
        if (bus.is_round_route) {
            delimiter = '>';
        } else {
            delimiter = '-';
        }
        CutPartOfRequest(text, ':');
        size_t end = text.find(delimiter);
        while (end != text.npos) {
            word = text.substr(0, end - 1);
            words.push_back(std::move(word));
            CutPartOfRequest(text, end);
            end = text.find(delimiter);
        }
        word = text.substr(0, text.size());
        words.push_back(std::move(word));
        return words;
    }

    string_view GetCommand(const string_view& request) {
        size_t pos = request.find(' ');
        string_view command = request.substr(0,pos);
        return command;
    }

    RequestType GetRequestType(const string_view& request) {
        string_view command = GetCommand(request);
        if (command == "Bus"sv) {
            return RequestType::BUS;
        } else {
            return RequestType::STOP;
        }
    }

    string GetNameFromRequest(string_view request) {
        size_t pos1 = request.find(' ') + 1;
        size_t pos2 = request.find(':');
        string name = string(request.substr(pos1, pos2 - pos1));
        CutPartOfRequest(request, pos2);
        return name;
    }

    bool GetRouteType(const string_view& request) {
        if (request.find('>') != string_view::npos) {
            return true;
        } else {
            return false;
        }
    }

    void SetDistanceBetweenStops(TransportCatalogue& catalogue, string_view request) {
        const Stop* from = catalogue.FindStop(GetNameFromRequest(request));
        CutPartOfRequest(request, ',');
        size_t start = request.find(',');
        if (start == request.npos) {
            return;
        } else {
            CutPartOfRequest(request, start);
            auto delimiter = request.find(',');
            while (delimiter != request.npos) {
                auto pos_m = request.find('m');
                int distance = stoi(string(request.substr(0, pos_m)));
                request.remove_prefix(pos_m + 5);
                delimiter = request.find(',');
                string destination;
                if (delimiter != request.npos) {
                    destination = string(request.substr(0, delimiter));
                    CutPartOfRequest(request, delimiter);
                } else {
                    destination = string(request.substr(0, request.size()));
                }
                const Stop* to = catalogue.FindStop(destination);
                catalogue.SetDistance(from, to, distance);
                delimiter = request.find(',');
            }
            auto pos_m = request.find('m');
            int distance = stoi(string(request.substr(0, pos_m)));
            request.remove_prefix(pos_m + 5);
            string destination = string(request.substr(0, request.size()));
            const Stop* to = catalogue.FindStop(destination);
            catalogue.SetDistance(from, to, distance);
        }
    }

    Stop GetStopFromRequest(string_view request) {
        Stop stop;
        stop.name = GetNameFromRequest(request);
        // Определяем широту из запроса
        CutPartOfRequest(request, ':');
        stop.coordinates.lat = stod(string(request.substr(0, request.find(','))));
        // Определяем долготу из запроса
        CutPartOfRequest(request, ',');
        auto end = request.find(',');
        if (end == request.npos) {
            stop.coordinates.lng = stod(string(request.substr(0, request.size())));
        } else {
            stop.coordinates.lng = stod(string(request.substr(0, end)));
            CutPartOfRequest(request, end);
        }
        return stop;
    }

    Bus GetBusFromRequest(TransportCatalogue& catalogue, const string_view& request) {
        Bus bus;
        bus.name = GetNameFromRequest(request);
        bus.is_round_route = GetRouteType(request);
        vector<string> bus_stops = SplitIntoStops(request, bus);
        // Отрабатываем случай когда маршрут не кольцевой
        if (!bus.is_round_route) {
            for (int i = (static_cast<int>(bus_stops.size()) - 2); i >= 0; --i) {
                bus_stops.push_back(bus_stops[i]);
            }
        }
        for (const auto& stop : bus_stops) {
            bus.stops.push_back(catalogue.FindStop(stop));
        }
        return bus;
    }

    void GetInputRequest(TransportCatalogue& catalogue, istream& input) {
        Requests requests;
        int request_count = detail_input::ReadLineWithNumber(input);
        string request;
        for(size_t i = 0; i < request_count; ++i) {
            getline(input, request);
            if (request.empty()) {
                continue;
            }
            RequestType request_type = GetRequestType(request);
            if(request_type == RequestType::STOP) {
                // Добавляем остановки в каталог
                catalogue.AddStop(GetStopFromRequest(request));
                requests.stops.push_back(std::move(request));
            } else {
                requests.buses.push_back(std::move(request));
            }
        }
        // Вносим фактические расстояния между остановками в каталог
        for(const auto& stop : requests.stops) {
            SetDistanceBetweenStops(catalogue, stop);
        }
        // Добавляем маршруты в каталог
        for(const auto& bus : requests.buses) {
            catalogue.AddBus(GetBusFromRequest(catalogue, bus));
        }
    }
}