#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/multi_index.hpp>

using namespace eosio;
using namespace std;

class[[eosio::contract("magiegateway")]] magiegateway : public eosio::contract
{
  public:
    magiegateway(name receiver, name code,  datastream<const char*> ds): contract(receiver, code, ds) {}
    void transfer(name from, name to, asset quantity, string memo);
    
    [[eosio::action]]
    void erase(int64_t id);

  private:
    void inlineAction_transfer(name to, asset currency, string memo)
    {
        action transfer_action = action(
            permission_level{_self, "active"_n},
            _code,
            "transfer"_n,
            std::make_tuple(_self, to, currency, memo));
        transfer_action.send();
    }

    void shareTax(asset tax, string recommender, string& info);

    struct [[eosio::table]] flowlog {
        uint64_t id;
        name owner;
        name user;
        bool isEnter;
        string info;
        uint64_t time;
        uint64_t primary_key() const { return id;}
    };
    typedef eosio::multi_index<"flowlog"_n, flowlog> flowlog_index;
    void insert(uint64_t id, bool isEnter, name owner, name user, string info, uint64_t time);

    struct [[eosio::table]] counter {
      name key;
      uint64_t all;
      uint64_t enter;
      uint64_t exit;
      uint64_t primary_key() const { return key.value; }
    };
    using count_index = eosio::multi_index<"counter"_n, counter>;
    uint64_t count(bool isEnter);
};

#define EOSIO_DISPATCH_CUSTOM( TYPE, MEMBERS ) \
extern "C" { \
    void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
        auto self = receiver; \
        bool self_action = code == self && action != name("transfer").value; \
        bool transfer = action == name("transfer").value; \
        if( action == name("onerror").value) { \
            /* onerror is only valid if it is for the "eosio" code account and authorized by "eosio"'s "active permission */ \
            eosio_assert(code == name("eosio").value, "onerror action's are only valid from the \"eosio\" system account"); \
        } \
        else if(self_action || transfer) { \
            switch( action ) { \
                EOSIO_DISPATCH_HELPER( TYPE, MEMBERS ) \
            } \
        } \
    } \
} \

EOSIO_DISPATCH_CUSTOM( magiegateway, (transfer)(erase) )

bool isMagieCoin(name isser, asset currency)
{
    if (isser == name("eosio.token") && currency.symbol == symbol("EOS", 4) && currency.amount >= 950)
    {
        return true;
    }
    if (isser == name("gamechaineos") && currency.symbol == symbol("GMC", 4) && currency.amount >= 95000)
    {
        return true;
    }
    if (isser == name("community123") && currency.symbol == symbol("COMC", 4) && currency.amount >= 95000)
    {
        return true;
    }
    return false;
}

bool isGameOwner(name account)
{
    if (account == name("magiecube111") 
    || account == name("gamemagiegmc")
    || account == name("reytheon1111")
    || account == name("leishendaoyu")
    || account == name("masterg11111")
    || account == name("mastergmagir")
    || account == name("demo"))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void SplitString(const string &s, vector<string> &v, const string &c) {
    string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while (string::npos != pos2) {
        v.push_back(s.substr(pos1, pos2 - pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if (pos1 != s.length())
        v.push_back(s.substr(pos1));
}

string connectString(vector<string> v, const string c){
    string newStr = v[0];
    int count = v.size();
    for (int i = 1; i < count;i++)
    {
        newStr += c;
        newStr +=v[i];
    }
    return newStr;
}