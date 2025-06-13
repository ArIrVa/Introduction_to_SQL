// Task 1

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Postgres.h>
#include <memory>
#include <string>

using namespace std;
namespace dbo = Wt::Dbo;

class Book;
class Stock;
class Sale;

class Publisher
{
public:
    std::string name;
    dbo::collection<dbo::ptr<Book>> books;

    template <class Action>
    void persist(Action &a)
    {
        dbo::field(a, name, "name");
        dbo::hasMany(a, books, dbo::ManyToOne, "publisher");
    }
};

class Shop
{
public:
    std::string name;
    dbo::collection<dbo::ptr<Stock>> stock;

    template <class Action>
    void persist(Action &a)
    {
        dbo::field(a, name, "name");
        dbo::hasMany(a, stock, dbo::ManyToOne, "shop");
    }
};

class Book
{
public:
    std::string title;
    dbo::ptr<Publisher> publisher;
    dbo::collection<dbo::ptr<Stock>> stock;

    template <class Action>
    void persist(Action &a)
    {
        dbo::field(a, title, "title");
        dbo::belongsTo(a, publisher, "publisher");
        dbo::hasMany(a, stock, dbo::ManyToOne, "book");
    }
};

class Stock
{
public:
    int count;
    dbo::collection<dbo::ptr<Sale>> sale;
    dbo::ptr<Book> book;
    dbo::ptr<Shop> shop;

    template <class Action>
    void persist(Action &a)
    {
        dbo::field(a, count, "count");
        dbo::belongsTo(a, book, "book");
        dbo::belongsTo(a, shop, "shop");
        dbo::hasMany(a, sale, dbo::ManyToOne, "stock");
    }
};

class Sale
{
public:
    int count;
    double price;
    std::string date_sale;
    dbo::ptr<Stock> stock;

    template <class Action>
    void persist(Action &a)
    {
        dbo::field(a, count, "count");
        dbo::field(a, price, "price");
        dbo::field(a, date_sale, "date_sale");
        dbo::belongsTo(a, stock, "stock");
    }
};

void run()
{
    std::string conn_str;

    conn_str = "host=localhost "
               "port=5432 "
               "dbname=bookstore "
               "user=postgres "
               "password=!1@2Epocsa";

    try
    {
        auto postgres = std::make_unique<dbo::backend::Postgres>(conn_str);
        dbo::Session session;
        session.setConnection(std::move(postgres));

        session.mapClass<Publisher>("publisher");
        session.mapClass<Shop>("shop");
        session.mapClass<Book>("book");
        session.mapClass<Stock>("stock");
        session.mapClass<Sale>("sale");

        Wt::Dbo::Transaction transaction{session};
        session.createTables();
        transaction.commit();
        cout << "OK!" << endl;
    }
    catch (const dbo::Exception &e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }
}

int main()
{
    run();

    return 0;
}