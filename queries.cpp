#include "queries.h"
#include "loginpage.h"

Queries::Queries(QObject *parent)
    : QObject{parent}
{
    //username = LoginPage::getUsername();
    username = LoginPage::getUser().getUsername();

    /*availableFriendsQuery = "SELECT " + username + "_friends.username_alias FROM " + username + "_friends " +
                "INNER JOIN users ON " + username + "_friends.id = users.id AND users.available = 1";*/

    /*unavailableFriendsQuery = "SELECT " + username + "_friends.username_alias FROM " + username + "_friends " +
                "INNER JOIN users ON " + username + "_friends.id = users.id AND users.available = 0";*/

    availableFriendsQuery = "SELECT * FROM " + username + "_friends " +
                    "INNER JOIN users ON " + username + "_friends.id = users.id AND users.available = 1";

    unavailableFriendsQuery = "SELECT * FROM " + username + "_friends " +
                    "INNER JOIN users ON " + username + "_friends.id = users.id AND users.available = 0";

    allFriendsQuery = "SELECT users.available FROM users INNER JOIN " + username +
            "_friends ON users.id = " + username + "_friends.id";

    newMessagesQuery = "SELECT is_new_message FROM " + username + "_friends ";
}
