#include "../src/client.hpp"

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <string>
#include <fmt/core.h>

ftxui::Component create_message(const std::string& message) {
    using namespace ftxui;
    auto transform = [](const EntryState& state) {
        auto sep = state.label.find_first_of(' ');
        std::string author = state.label.substr(0, sep);
        std::string&& message = state.label.substr(sep);

        bool mine = author == "moi";
        Element msg = window(text(std::move(author)) | bold | underlined, paragraph(message));
        if (state.active) { msg |= inverted; }
        if (mine) {
            return hbox({ msg, filler() });
        } else {
            return hbox({ filler(), msg });
        }
    };
    static MenuEntryOption option{transform, {}};

    return MenuEntry(message, option);
}

void receive_handler(Client& client, ftxui::ScreenInteractive& screen, ftxui::Component& messages) {
    std::string message;
    while (client.receive(message) && message != "stop") {
        auto sep = message.find_first_of(' ');
        messages->Add(create_message(message));
        screen.PostEvent(ftxui::Event::Custom);
    }
}

bool send_handler(Client& client, std::string_view msg) {
    if (!msg.empty() && msg[0] == '!') {
        client.log("Message special!");
        client.log(msg);
        msg = msg.substr(1);
        auto sep = msg.find_first_of(' ');
        auto prefix = msg.substr(0, sep);
        if (prefix == "username") {
            client.set_username(msg.substr(sep + 1));
        }
        return false;
    } else {
        client.send(msg);
        return true;
    }
}

int main(int argc, char* argv[]) {
    using namespace ftxui;
 
    Client client;
    client.set_log("log.txt");
    std::thread receiver;

    auto screen = ScreenInteractive::Fullscreen();

    int selected = 0;
    auto messages = Container::Vertical({}, &selected);
    std::string error_message;

    auto disconnect = [&]() {
        if (client.is_connected()) {
            client.send("stop");
            receiver.join();
            client.disconnect();
        }
    };

    auto connect = [&](std::string_view server_address) {
        disconnect();
        error_message = "Tentative de connexion ...";
        screen.PostEvent(Event::Custom);
        if (client.connect(server_address)) {
            receiver = std::thread(receive_handler, std::ref(client), std::ref(screen), std::ref(messages));
            error_message.clear();
        } else {
            error_message = "Echec de la connexion";
        }
    };

    auto error = Maybe(Renderer([&](){ return paragraph(error_message) | color(Color::Red1); }), [&](){ return !error_message.empty(); });

    std::string message;
    InputOption option;
    option.on_enter = [&](){
        if (send_handler(client, message)) {
            messages->Add(create_message(fmt::format("moi {}", message)));
        }
        message = "";
    };
    auto message_input = Input(&message, "> ", option);

    std::string server_address;
    auto server_input = Input(&server_address, "");

    auto connect_button = Button("Connexion", [&]() { connect(server_address); });
    auto disconnect_button = Button("Deconnexion", disconnect);

    auto quit_button = Button("Quitter", screen.ExitLoopClosure());

    auto messages_frame = Renderer(messages, [&]() {
        if (messages->ChildCount() > 0) {
            return messages->Render();
        } else {
            return emptyElement();
        }
    });

    auto components = Container::Horizontal({
        Container::Vertical({
            messages_frame,
            message_input
        }),
        Container::Vertical({
            server_input,
            connect_button,
            disconnect_button,
            quit_button,
            error
        })
    });

    auto component = Renderer(components, [&](){
        return hbox({
            vbox({
                messages_frame->Render() | vscroll_indicator | yframe | size(Direction::HEIGHT, Constraint::EQUAL, Terminal::Size().dimy - 2),
                filler(),
                separator(),
                message_input->Render()
            }) | flex,
            separator(),
            vbox({
                vbox({
                    text("Adresse du serveur :"),
                    server_input->Render(),
                    connect_button->Render(),
                    disconnect_button->Render(),
                    separator(),
                    error->Render()
                }) | border,
                quit_button->Render()
            }) | flex_shrink
        });
    });
    
    screen.Loop(component);

    client.disconnect();
    if (receiver.joinable()) {
        receiver.join();
    }

    return 0;
}
