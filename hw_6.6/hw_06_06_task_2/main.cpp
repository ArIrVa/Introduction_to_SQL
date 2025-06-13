// Task 2

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

void AddPublisher(const std::string &text, dbo::Session &ses)
{
    auto publisher = std::make_unique<Publisher>();
    publisher->name = text;
    dbo::ptr<Publisher> Ptr = ses.add(std::move(publisher));
}

void AddShop(const std::string &text, dbo::Session &ses)
{
    auto shop = std::make_unique<Shop>();
    shop->name = text;
    dbo::ptr<Shop> Ptr = ses.add(std::move(shop));
}

void AddBook(const std::string &text, dbo::ptr<Publisher> &publisher, dbo::Session &ses)
{
    auto book = std::make_unique<Book>();
    book->title = text;
    book->publisher = publisher;
    dbo::ptr<Book> Ptr = ses.add(std::move(book));
}

void AddStock(const int &count, dbo::ptr<Book> &book, dbo::ptr<Shop> &shop, dbo::Session &ses)
{
    auto stock = std::make_unique<Stock>();
    stock->count = count;
    stock->book = book;
    stock->shop = shop;
    dbo::ptr<Stock> Ptr = ses.add(std::move(stock));
}

void AddSale(const int &count, const double &price, std::string date_sale, dbo::ptr<Stock> stock, dbo::Session &ses)
{
    auto sale = std::make_unique<Sale>();
    sale->count = count;
    sale->price = price;
    sale->date_sale = date_sale;
    sale->stock = stock;
    dbo::ptr<Sale> Ptr = ses.add(std::move(sale));
}

bool isTablesExists(dbo::Session &ses)
{
    int count = ses.query<int>("select count(1) from pg_catalog.pg_tables")
                    .where("schemaname = ?")
                    .bind("public");
    if (count)
    {
        return true;
    }
    else
        return false;
}

void PrintStockPublisherInfo(const std::string &publisher, dbo::Session &ses)
{
    dbo::Query<dbo::ptr<Shop>, dbo::DynamicBinding> Result;

    bool isName{false};

    for (const auto &pch : publisher)
    {
        if (isalpha(pch))
        {
            isName = true;
            break;
        }
    }

    if (isName)
    {
        Result = ses.query<dbo::ptr<Shop>>("select s from public.shop s")
                     .join<Stock>("st", "st.shop_id = s.id")
                     .join<Book>("b", "b.id = st.book_id")
                     .join<Publisher>("p", "p.id = b.publisher_id")
                     .groupBy("s.id")
                     .where("p.name = ?")
                     .bind(publisher);
    }
    else
    {
        Result = ses.query<dbo::ptr<Shop>>("select s from public.shop s")
                     .join<Stock>("st", "st.shop_id = s.id")
                     .join<Book>("b", "b.id = st.book_id")
                     .join<Publisher>("p", "p.id = b.publisher_id")
                     .groupBy("s.id")
                     .where("p.id = ?")
                     .bind(stoi(publisher));
    }

    for (auto el : Result.resultList())
    {
        cout << el->name << endl;
    }
}

void run()
{
    using Publishers = dbo::collection<dbo::ptr<Publisher>>;
    using Shops = dbo::collection<dbo::ptr<Shop>>;
    using Books = dbo::collection<dbo::ptr<Book>>;
    using Stocks = dbo::collection<dbo::ptr<Stock>>;
    using Sales = dbo::collection<dbo::ptr<Sale>>;

    std::string conn_str;
    dbo::ptr<Publisher> ptrPublisher;
    dbo::ptr<Book> ptrBook;
    dbo::ptr<Shop> ptrShop;
    dbo::ptr<Stock> ptrStock;

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

        if (isTablesExists(session))
        {
            session.dropTables();
        }

        session.createTables();

        AddShop("Book-home", session);
        AddShop("Biblio Globus", session);
        AddShop("Labirint", session);

        AddPublisher("Prosveschenie", session);
        AddPublisher("AST", session);
        AddPublisher("Binom", session);
        AddPublisher("Miph", session);
        AddPublisher("Ecsmo", session);

        ptrPublisher = session.find<Publisher>().where("name = ?").bind("Prosveschenie");
        AddBook("Code", ptrPublisher, session);
        AddBook("The world around you", ptrPublisher, session);
        AddBook("Mathematics", ptrPublisher, session);
        AddBook("Psychology", ptrPublisher, session);

        ptrPublisher = session.find<Publisher>().where("name = ?").bind("AST");
        AddBook("Mybook", ptrPublisher, session);
        AddBook("Cookbook", ptrPublisher, session);
        AddBook("The notebook of life", ptrPublisher, session);

        ptrPublisher = session.find<Publisher>().where("name = ?").bind("Binom");
        AddBook("Fundamentals of programming", ptrPublisher, session);
        AddBook("The basics of security", ptrPublisher, session);

        ptrPublisher = session.find<Publisher>().where("name = ?").bind("Miph");
        AddBook("Mythology", ptrPublisher, session);
        AddBook("Fantastic worlds", ptrPublisher, session);

        ptrPublisher = session.find<Publisher>().where("name = ?").bind("Ecsmo");
        AddBook("Fairy tales", ptrPublisher, session);

        ptrShop = session.find<Shop>().where("name = ?").bind("Book-home");

        ptrBook = session.find<Book>().where("title = ?").bind("Mybook");
        AddStock(3, ptrBook, ptrShop, session);
        ptrBook = session.find<Book>().where("title = ?").bind("Cookbook");
        AddStock(15, ptrBook, ptrShop, session);
        ptrBook = session.find<Book>().where("title = ?").bind("The notebook of life");
        AddStock(25, ptrBook, ptrShop, session);
        ptrBook = session.find<Book>().where("title = ?").bind("Psychology");
        AddStock(3, ptrBook, ptrShop, session);
        ptrBook = session.find<Book>().where("title = ?").bind("Fairy tales");
        AddStock(10, ptrBook, ptrShop, session);

        ptrShop = session.find<Shop>().where("name = ?").bind("Biblio Globus");

        ptrBook = session.find<Book>().where("title = ?").bind("Code");
        AddStock(5, ptrBook, ptrShop, session);
        ptrBook = session.find<Book>().where("title = ?").bind("Mathematics");
        AddStock(10, ptrBook, ptrShop, session);
        ptrBook = session.find<Book>().where("title = ?").bind("Psychology");
        AddStock(8, ptrBook, ptrShop, session);
        ptrBook = session.find<Book>().where("title = ?").bind("Fundamentals of programming");
        AddStock(5, ptrBook, ptrShop, session);
        ptrBook = session.find<Book>().where("title = ?").bind("The basics of security");
        AddStock(3, ptrBook, ptrShop, session);
        ptrBook = session.find<Book>().where("title = ?").bind("The notebook of life");
        AddStock(7, ptrBook, ptrShop, session);
        ptrBook = session.find<Book>().where("title = ?").bind("The world around you");
        AddStock(15, ptrBook, ptrShop, session);
        ptrBook = session.find<Book>().where("title = ?").bind("Cookbook");
        AddStock(10, ptrBook, ptrShop, session);

        ptrShop = session.find<Shop>().where("name = ?").bind("Labirint");

        ptrBook = session.find<Book>().where("title = ?").bind("Mythology");
        AddStock(10, ptrBook, ptrShop, session);
        ptrBook = session.find<Book>().where("title = ?").bind("Fantastic worlds");
        AddStock(23, ptrBook, ptrShop, session);
        ptrBook = session.find<Book>().where("title = ?").bind("Fairy tales");
        AddStock(20, ptrBook, ptrShop, session);
        ptrBook = session.find<Book>().where("title = ?").bind("Psychology");
        AddStock(5, ptrBook, ptrShop, session);
        ptrBook = session.find<Book>().where("title = ?").bind("The notebook of life");
        AddStock(5, ptrBook, ptrShop, session);

        ptrStock = session.find<Stock>().where("book_id = ?").bind(4).where("shop_id = ?").bind(1);
        AddSale(5, 250, "2025-02-20", ptrStock, session);
        ptrStock = session.find<Stock>().where("book_id = ?").bind(5).where("shop_id = ?").bind(1);
        AddSale(1, 190, "2025-03-15", ptrStock, session);
        ptrStock = session.find<Stock>().where("book_id = ?").bind(6).where("shop_id = ?").bind(1);
        AddSale(1, 250, "2025-04-12", ptrStock, session);
        ptrStock = session.find<Stock>().where("book_id = ?").bind(7).where("shop_id = ?").bind(1);
        AddSale(15, 300, "2025-04-10", ptrStock, session);
        ptrStock = session.find<Stock>().where("book_id = ?").bind(12).where("shop_id = ?").bind(1);
        AddSale(15, 300, "2025-02-10", ptrStock, session);

        ptrStock = session.find<Stock>().where("book_id = ?").bind(4).where("shop_id = ?").bind(2);
        AddSale(8, 230, "2025-02-10", ptrStock, session);
        ptrStock = session.find<Stock>().where("book_id = ?").bind(8).where("shop_id = ?").bind(2);
        AddSale(4, 380, "2025-04-13", ptrStock, session);
        ptrStock = session.find<Stock>().where("book_id = ?").bind(9).where("shop_id = ?").bind(2);
        AddSale(3, 350, "2025-04-17", ptrStock, session);
        ptrStock = session.find<Stock>().where("book_id = ?").bind(6).where("shop_id = ?").bind(2);
        AddSale(1, 200, "2025-03-17", ptrStock, session);
        ptrStock = session.find<Stock>().where("book_id = ?").bind(7).where("shop_id = ?").bind(2);
        AddSale(1, 310, "2025-02-22", ptrStock, session);
        ptrStock = session.find<Stock>().where("book_id = ?").bind(2).where("shop_id = ?").bind(2);
        AddSale(2, 150, "2025-03-11", ptrStock, session);

        ptrStock = session.find<Stock>().where("book_id = ?").bind(10).where("shop_id = ?").bind(3);
        AddSale(8, 200, "2025-03-20", ptrStock, session);
        ptrStock = session.find<Stock>().where("book_id = ?").bind(11).where("shop_id = ?").bind(3);
        AddSale(20, 220, "2025-04-18", ptrStock, session);
        ptrStock = session.find<Stock>().where("book_id = ?").bind(12).where("shop_id = ?").bind(3);
        AddSale(20, 300, "2025-02-15", ptrStock, session);
        ptrStock = session.find<Stock>().where("book_id = ?").bind(4).where("shop_id = ?").bind(3);
        AddSale(5, 210, "2025-04-10", ptrStock, session);
        ptrStock = session.find<Stock>().where("book_id = ?").bind(7).where("shop_id = ?").bind(3);
        AddSale(3, 320, "2025-03-01", ptrStock, session);

        std::string publisher_value = "";

        cout << "Input name publisher or id: ";
        cin >> publisher_value;
        cout << "----------" << endl;
        PrintStockPublisherInfo(publisher_value, session);

        transaction.commit();

        cout << endl;
        cout << "Done!" << endl;
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