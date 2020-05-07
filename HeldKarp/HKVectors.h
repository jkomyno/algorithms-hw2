#pragma once
#include <unordered_map>
#include <unordered_set>

//class for representing the d vector
template <typename T = int>
class HKVectors {

 
    struct hash_set {
        template <class T1> 
        size_t operator()(const std::unordered_set<T1>& set) const {

            size_t hash1 = 0;

            for (size_t s : set) {
                hash1 = hash1 ^ std::hash<T1>{}(s);
            }

            return hash1;
        }
    };

    std::unordered_map<size_t, std::unordered_map<std::unordered_set<size_t>, std::pair<T, size_t>, hash_set>> hk;

    
    public:
    bool is_already_calculated(size_t v, std::unordered_set<size_t> S) {

        if (hk.count(v) == 1 && hk.at(v).count(S) == 1) {
            return true;
        }

        return false;
    }

    T get_distance(size_t v, std::unordered_set<size_t> S) {
        return hk.at(v).at(S).first;
    }

    size_t get_predec(size_t v, std::unordered_set<size_t> S) {
        return hk.at(v).at(S).second;
    }

    void set_distance(size_t v, std::unordered_set<size_t> S, T distance) {
        hk[v][S].first = distance;
    }

    void set_predec(size_t v, std::unordered_set<size_t> S, size_t pred) {
        hk[v][S].second = pred;
    }

};
