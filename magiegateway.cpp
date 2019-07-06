#include "magiegateway.hpp"

void magiegateway::transfer(name from, name to, asset quantity, string memo)
{
    require_auth(from);
    if (from == _self || to != _self)
    {
        return;
    }
    eosio_assert(isMagieCoin(quantity.symbol, _code), "I don't want these coins.");

    // remove space
    memo.erase(std::remove_if(memo.begin(),
                              memo.end(),
                              [](unsigned char x) { return std::isspace(x); }),
               memo.end());
    size_t sep_count_enter = std::count(memo.begin(), memo.end(), '<');
    size_t sep_count_exit = std::count(memo.begin(), memo.end(), '>');

    string recommender;
    asset tax;
    bool isEnter;
    name target;

    //enter box
    if (sep_count_enter == 1 && sep_count_exit == 0)
    {
        isEnter = true;
        // tax = asset(0, quantity.symbol);
        tax = quantity / 50; // tax 2%
        vector<string> memo_v;
        SplitString(memo, memo_v, "<");
        vector<string> head_v;
        SplitString(memo_v[0], head_v, "-");
        eosio_assert(head_v.size() == 2, "invalid memo head.");
        target = name(head_v[0]);
        eosio_assert(isGameOwner(target), "<-- invalid GameOwner.");
        eosio_assert(!isGameOwner(from), "<-- invalid player.");
        recommender = head_v[1];
        eosio_assert(from != name(recommender), "referrer can not be your self.");
        head_v[0] = from.to_string();
        memo_v[0] = connectString(head_v, "-");
        string newMemo = connectString(memo_v, ">");
        print(", newMemo:", newMemo);
        inlineAction_transfer(target, quantity - tax, newMemo);
    }
    //exit box
    else if (sep_count_exit == 1 && sep_count_enter == 0)
    {
        isEnter = false;
        // tax = asset(0, quantity.symbol);
        tax = quantity / 50; // tax 2%
        vector<string> memo_v;
        SplitString(memo, memo_v, ">");
        vector<string> head_v;
        SplitString(memo_v[0], head_v, "-");
        eosio_assert(head_v.size() == 2, "invalid memo head.");
        eosio_assert(isGameOwner(from), "--> invalid GameOwner.");
        target = name(head_v[0]);
        eosio_assert(!isGameOwner(target), "--> invalid player.");
        string target = head_v[0];
        recommender = head_v[1];
        memo_v[0] = from.to_string();
        string newMemo = connectString(memo_v, ">");
        print(", newMemo:", newMemo);
        inlineAction_transfer(name(target), quantity - tax, newMemo);
    }
    else
    {
        eosio_assert(false, "invalid box memo.");
    }

    string info;
    name owner, user;
    if (isEnter)
    {
        owner = target;
        user = from;
        info = owner.to_string() + " <-- " + user.to_string() + " " + quantity.to_string();
    }
    else
    {
        owner = from;
        user = target;
        info = owner.to_string() + " --> " + user.to_string() + " " + quantity.to_string();
    }

    info += ": " + target.to_string() + " " + (quantity - tax).to_string();

    if (tax.amount > 0)
        shareTax(tax, recommender, info);

    uint64_t id = count(isEnter);
    insert(id, isEnter, owner, user, info, now());
}

void magiegateway::shareTax(asset tax, string recommender, string &info)
{
    asset inStorage = tax;

    // share out bonus 40%
    asset temp = tax * 2 / 5;
    inlineAction_transfer("boxawardpool"_n, temp, "share out bonus.");
    info += ", boxawardpool " + temp.to_string();
    inStorage -= temp;
    // return commision 20%
    temp = tax / 5;
    inlineAction_transfer(name(recommender), temp, "return commision.");
    info += ", " + recommender + " " + temp.to_string();

    inStorage -= temp;
    inlineAction_transfer("eosmagiebank"_n, inStorage, "be put in storage.");
    info += ", eosmagiebank " + inStorage.to_string();
}

void magiegateway::erase(int64_t id)
{
    require_auth(_self);
    flowlog_index log(_self, _self.value);
    auto it = log.find(id);
    eosio_assert(it != log.end(), "Record does not exist.");
    while (true)
    {
        if (it == log.begin())
        {
            log.erase(it);
            break;
        }
        else
        {
            log.erase(it--);
        }
    }
}

void magiegateway::insert(uint64_t id, bool isEnter, name owner, name user, string info, uint64_t time)
{
    flowlog_index log(_self, _self.value);
    log.emplace(_self, [&](auto &row) {
        row.id = id;
        row.isEnter = isEnter;
        row.owner = owner;
        row.user = user;
        row.info = info;
        row.time = time;
    });
}

uint64_t magiegateway::count(bool isEnter)
{
    count_index counts(_self, _self.value);
    auto iterator = counts.find(name("counter").value);
    uint64_t all = 1;
    if (iterator == counts.end())
    {
        counts.emplace(_self, [&](auto &row) {
            row.key = name("counter");
            row.all = 1;
            row.enter = (isEnter) ? 1 : 0;
            row.exit = (!isEnter) ? 1 : 0;
        });
    }
    else
    {
        counts.modify(iterator, _self, [&](auto &row) {
            row.all += 1;
            all = row.all;
            if (isEnter)
            {
                row.enter += 1;
            }
            else
            {
                row.exit += 1;
            }
        });
    }
    return all;
}

bool magiegateway::isMagieCoin(symbol sb, name issuer)
{
    cointype_index cointypes(name("magiesupport"), name("magiesupport").value);
    auto iterator = cointypes.find(sb.code().raw());
    if (iterator != cointypes.end() && iterator->symbol == sb)
        return true;
    return false;
}
bool magiegateway::isGameOwner(name account)
{
    gameowner_index gameowners(name("magiesupport"), name("magiesupport").value);
    auto iterator = gameowners.find(account.value);
    if (iterator != gameowners.end())
        return true;
    return false;
}
