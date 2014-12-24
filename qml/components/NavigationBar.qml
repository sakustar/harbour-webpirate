import QtQuick 2.0
import Sailfish.Silica 1.0
import "../js/Database.js" as Database
import "../js/Favorites.js" as Favorites

Item
{
    signal backRequested()
    signal refreshRequested()
    signal stopRequested()
    signal forwardRequested()
    signal searchRequested(string searchquery)

    property bool favorite: false

    property alias searchBar: searchbar
    property alias backButton: btnback
    property alias forwardButton: btnforward

    id: navigationbar
    height: visible ? 60 : 0
    state: "loaded";

    states: [ State { name: "loaded"; PropertyChanges { target: btnrefresh; icon.source: "image://theme/icon-m-refresh" } },
              State { name: "loading"; PropertyChanges { target: btnrefresh; icon.source: "image://theme/icon-m-reset" } } ]

    Behavior on height {
        NumberAnimation { duration: 100; easing.type: Easing.InOutQuad }
    }

    IconButton
    {
        id: btnback
        icon.source: "image://theme/icon-m-back"
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        anchors.verticalCenter: navigationbar.verticalCenter
        anchors.left: navigationbar.left
        enabled: false

        onClicked: backRequested();
    }

    IconButton
    {
        id: btnhome
        icon.source: "image://theme/icon-m-home"
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        anchors.verticalCenter: navigationbar.verticalCenter
        anchors.left: btnback.right

        onClicked: navigationbar.searchRequested(mainwindow.settings.homepage)
    }

    IconButton
    {
        id: btnbookmark
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        icon.source: (favorite ? "image://theme/icon-m-favorite-selected" : "image://theme/icon-m-favorite")
        anchors.verticalCenter: navigationbar.verticalCenter
        anchors.left: btnhome.right

        onClicked: {
            if(!favorite) {
                Favorites.add(Database.instance(), mainwindow.settings.favorites, webview.title, webview.url.toString(), icon.source.toString());
                navigationbar.favorite = true;
            }
            else {
                Favorites.remove(Database.instance(), mainwindow.settings.favorites, webview.url.toString());
                navigationbar.favorite = false;
            }
        }
    }

    SearchBar
    {
        id: searchbar
        anchors.left: btnbookmark.right
        anchors.right: btnrefresh.left

        onReturnPressed: navigationbar.searchRequested(searchquery);
    }

    IconButton
    {
        id: btnrefresh
        icon.source: "image://theme/icon-m-refresh"
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        anchors.verticalCenter: navigationbar.verticalCenter
        anchors.right: btnforward.left

        onClicked: {
            if(navigationbar.state === "loaded")
                refreshRequested();
            else if(navigationbar.state === "loading")
                stopRequested();
        }
    }

    IconButton
    {
        id: btnforward
        icon.source: "image://theme/icon-m-forward"
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        anchors.verticalCenter: navigationbar.verticalCenter
        anchors.right: navigationbar.right
        enabled: false

        onClicked: forwardRequested();
    }
}
