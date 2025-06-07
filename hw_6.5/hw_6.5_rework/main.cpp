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

    void dbCreate()
    {
        pqxx::work tx{cn};

        tx.exec("CREATE TABLE client ("
                "id_client SERIAL primary key,"
                "name text,"
                "surname text,"
                "email text );");

        tx.exec("CREATE TABLE phone ("
                "phone text,"
                "id_client integer references client (id_client),"
                "primary key (id_client, phone) );");

        tx.commit();
    }

    void addClient(const std::string &name, const std::string &surname, const std::string &email)
    {
        pqxx::work tx{cn};
        tx.exec("INSERT INTO public.client(name, surname, email)"
                "VALUES ('" +
                name + "', '" + surname + "', '" + email + "');");
        tx.commit();
    }

    void updateClient(const int &idClient, const std::string &name, const std::string &surname, const std::string &email)
    {
        pqxx::work tx{cn};
        tx.exec("UPDATE public.client "
                "SET name = '" +
                name + "', " + "surname = '" + surname + "', email = '" + email + "'"
                                                                                  "WHERE id_client = " +
                to_string(idClient) + ";");
        tx.commit();
    }

    void deleteClient(const int &idClient)
    {
        pqxx::work tx{cn};
        tx.exec("DELETE FROM public.phone "
                "WHERE id_client = " +
                to_string(idClient) + ";");
        tx.exec("DELETE FROM public.client "
                "WHERE id_client = " +
                to_string(idClient) + ";");
        tx.commit();
    }

    void addPhoneNumber(const int &idClient, const std::string &phoneNumber)
    {
        pqxx::work tx{cn};
        tx.exec("INSERT INTO public.phone(id_client, phone)"
                "VALUES (" +
                to_string(idClient) + ", '" + phoneNumber + "');");
        tx.commit();
    }

    void deletePhoneNumber(const int &idClient, const std::string &phoneNumber)
    {
        pqxx::work tx{cn};
        tx.exec("DELETE FROM public.phone "
                "WHERE id_client = " +
                to_string(idClient) + " AND phone = '" + phoneNumber + "';");
        tx.commit();
    }

    std::vector<std::string> findClient(const std::string &email)
    {
        std::vector<std::string> result;
        std::string text;
        pqxx::work tx{cn};
        pqxx::result res_client = tx.exec("SELECT name, surname, id_client "
                                          "FROM public.client "
                                          "WHERE  email = '" +
                                          email + "'; ");
        pqxx::result res_phone = tx.exec("SELECT phone "
                                         "FROM public.phone "
                                         "WHERE  id_client = " +
                                         to_string(res_client[0][2]) + "; ");

        text = res_client[0][0].as<std::string>() + " " + res_client[0][1].as<std::string>();
        result.push_back(text);
        text = "";
        for (const auto &ph : res_phone)
        {
            if (text != "")
            {
                text += ", " + ph[0].as<std::string>();
            }
            else
            {
                text += ph[0].as<std::string>();
            }
        }
        result.push_back(text);
        return result;
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
        std::vector<std::string> clientInfo;

        Client_manager cm(conn);
        // cm.dbCreate();

        // cm.addClient("name1", "surname2", "email1");
        // cm.addClient("name2", "surname2", "email2");
        // cm.addClient("name3", "surname3", "email3");

        // cm.addPhoneNumber(1, "123-22-33");
        // cm.addPhoneNumber(1, "222-33-44");
        // cm.addPhoneNumber(1, "333-44-55");
        // cm.addPhoneNumber(2, "777-66-55");
        // cm.addPhoneNumber(4, "888-77-66");
        // cm.addPhoneNumber(4, "889-78-65");

        // clientInfo = cm.findClient("email1");
        // cout << clientInfo[0] << endl;
        // cout << clientInfo[1] << endl;
        // cout << clientInfo[0] << "; " << clientInfo[1] << endl;

        // cm.updateClient(2, "name_second", "surname_second", "email_second");
        // cm.updateClient(2, "name2", "surname2", "email2");
        // cm.updateClient(1,"name1", "surname1", "email1");
        // cm.updateClient(4, "name4", "surname4", "email4");

        // cm.deleteClient(4);

        // cm.deletePhoneNumber(1, "222-33-44");

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