#include <iostream>
#include <memory>
#include <pqxx/pqxx>
#include <pqxx/except>
#include <exception>

using namespace std;

class Client_manager
{
public:
    Client_manager(pqxx::connection &c) : cn{c} {}
    ~Client_manager()
    {
        if (cn.is_open())
        {
            cn.close();
        }
    }

    void create_db()
    {
        pqxx::work tx{cn};

        tx.exec("CREATE TABLE client ("
                "name text,"
                "surname text,"
                "email text,"
                "PRIMARY KEY (name, surname) );");

        tx.exec("CREATE TABLE phone ("
                "phone text,"
                "name text,"
                "surname text,"
                "FOREIGN KEY (name, surname) REFERENCES client(name, surname),"
                "PRIMARY KEY (phone,name,surname) );");

        tx.commit();
    }

    void add_client(const std::string &na, const std::string &sna, const std::string &em)
    {
        pqxx::work tx{cn};
        tx.exec("INSERT INTO public.client(name, surname, email)"
                "VALUES ('" +
                na + "', '" + sna + "', '" + em + "');");
        tx.commit();
    }

    void add_phone(const std::string &ph, const std::string &na, const std::string &sna)
    {
        pqxx::work tx{cn};
        tx.exec("INSERT INTO public.phone(phone, name, surname)"
                "VALUES ('" +
                ph + "', '" + na + "', '" + sna + "');");
        tx.commit();
    }

    void change_data(const std::string &old_ph, const std::string &new_ph, const std::string &em, const std::string &na, const std::string &sna)
    {
        pqxx::work tx{cn};
        tx.exec("UPDATE public.client "
                "SET email = '" +
                em + "' "
                     "WHERE name = '" +
                na + "' AND surname = '" + sna + "';");
        tx.exec("UPDATE public.phone "
                "SET phone = '" +
                new_ph + "' "
                         "WHERE name = '" +
                na + "' AND surname = '" + sna + "' AND phone = '" + old_ph + "';");
        tx.commit();
    }

    void delete_phone(const std::string &ph, const std::string &na, const std::string &sna)
    {
        pqxx::work tx{cn};
        tx.exec("DELETE FROM public.phone "
                "WHERE phone = '" +
                ph + "' AND name = '" + na + "' AND surname = '" + sna + "';");
        tx.commit();
    }

    void delete_client(const std::string &na, const std::string &sna)
    {
        pqxx::work tx{cn};
        tx.exec("DELETE FROM public.phone "
                "WHERE name = '" +
                na + "' AND surname = '" + sna + "';");
        tx.exec("DELETE FROM public.client "
                "WHERE name = '" +
                na + "' AND surname = '" + sna + "';");
        tx.commit();
    }

    void find_client(const std::string &param)
    {
        pqxx::work tx{cn};
        pqxx::result res = tx.exec("SELECT t1.name, t1.surname "
                                   "FROM public.client AS t1, public.phone AS t2 "
                                   "WHERE t1.name = t2.name AND t1.surname = t2.surname "
                                   "AND (t1.name = '" +
                                   param + "' OR t1.surname = '" + param + "' OR phone = '" + param + "' OR email = '" + param + "' ); ");
        cout << res[0][0].c_str() << " " << res[0][1].c_str() << endl;
    }

private:
    pqxx::connection &cn;
};

int main()
{
    std::string conn_str;

    conn_str = "host=localhost "
               "port=5432 "
               "dbname=clients "
               "user=postgres "
               "password=~123!";

    try
    {
        pqxx::connection conn(conn_str);

        Client_manager cm(conn);
        // cm.create_db();
        // cm.add_client("name2", "surname2", "email2");
        // cm.add_phone("123-22-33", "name2", "surname2");
        // cm.add_phone("222-33-44", "name2", "surname2");
        // cm.change_data("111-22-33","111-22-44","email4","name1","surname1");
        // cm.delete_phone("777-44-55","name1","surname1");
        // cm.delete_client("name", "surname");
        // cm.find_client("email2");
        cout << "OK!" << endl;
    }
    catch (pqxx::sql_error e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}