#ifndef K_UI_H_
#define K_UI_H_

namespace K {
  string A();
  static uWS::Hub hub(0, false);
  typedef json (*uiCb)(json);
  struct uiSess { map<string, uiCb> cb; map<uiTXT, vector<json>> D; int u = 0; };
  static uWS::Group<uWS::SERVER> *uiGroup = hub.createGroup<uWS::SERVER>(uWS::PERMESSAGE_DEFLATE);
  static bool uiVisibleOpt = true;
  static unsigned int uiOSR_1m = 0;
  static unsigned long uiT_MKT = 0;
  static unsigned long uiT_1m = 0;
  static unsigned int uiThread = 0;
  static double ui_delayUI = 0;
  static string uiNOTE = "";
  static string uiNK64 = "";
  class UI {
    public:
      static void main() {
        CF::internal();
        int port = stoi(CF::cfString("WebClientListenPort"));
        string name = CF::cfString("WebClientUsername");
        string key = CF::cfString("WebClientPassword");
        uiGroup->setUserData(new uiSess);
        uiSess *sess = (uiSess *) uiGroup->getUserData();
        if (name != "NULL" && key != "NULL" && name.length() > 0 && key.length() > 0) {
          B64::Encode(name.append(":").append(key), &uiNK64);
          uiNK64 = string("Basic ").append(uiNK64);
        }
        uiGroup->onConnection([sess](uWS::WebSocket<uWS::SERVER> *webSocket, uWS::HttpRequest req) {
          sess->u++;
          typename uWS::WebSocket<uWS::SERVER>::Address address = webSocket->getAddress();
          cout << FN::uiT() << to_string(sess->u) << " UI currently connected, last connection was from " << address.address << endl;
        });
        uiGroup->onDisconnection([sess](uWS::WebSocket<uWS::SERVER> *webSocket, int code, char *message, size_t length) {
          sess->u--;
          typename uWS::WebSocket<uWS::SERVER>::Address address = webSocket->getAddress();
          cout << FN::uiT() << to_string(sess->u) << " UI currently connected, last disconnection was from " << address.address << endl;
        });
        uiGroup->onHttpRequest([&](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t length, size_t remainingBytes) {
          string document;
          string auth = req.getHeader("authorization").toString();
          typename uWS::WebSocket<uWS::SERVER>::Address address = res->getHttpSocket()->getAddress();
          if (uiNK64 != "" && auth == "") {
            cout << FN::uiT() << "UI authorization attempt from " << address.address << endl;
            document = "HTTP/1.1 401 Unauthorized\r\nWWW-Authenticate: Basic realm=\"Basic Authorization\"\r\nConnection: keep-alive\r\nAccept-Ranges: bytes\r\nVary: Accept-Encoding\r\nContent-Type:text/plain; charset=UTF-8\r\nContent-Length: 0\r\n\r\n";
            res->write(document.data(), document.length());
          } else if (uiNK64 != "" && auth != uiNK64) {
            cout << FN::uiT() << "UI authorization failed from " << address.address << endl;
            document = "HTTP/1.1 403 Forbidden\r\nConnection: keep-alive\r\nAccept-Ranges: bytes\r\nVary: Accept-Encoding\r\nContent-Type:text/plain; charset=UTF-8\r\nContent-Length: 0\r\n\r\n";
            res->write(document.data(), document.length());
          } else if (req.getMethod() == uWS::HttpMethod::METHOD_GET) {
            string url = "";
            document = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nAccept-Ranges: bytes\r\nVary: Accept-Encoding\r\nCache-Control: public, max-age=0\r\n";
            string path = req.getUrl().toString();
            string::size_type n = 0;
            while ((n = path.find("..", n)) != string::npos) path.replace(n, 2, "");
            const string leaf = path.substr(path.find_last_of('.')+1);
            if (leaf == "/") {
              cout << FN::uiT() << "UI authorization success from " << address.address << endl;
              document.append("Content-Type: text/html; charset=UTF-8\r\n");
              url = "/index.html";
            } else if (leaf == "js") {
              document.append("Content-Type: application/javascript; charset=UTF-8\r\nContent-Encoding: gzip\r\n");
              url = path;
            } else if (leaf == "css") {
              document.append("Content-Type: text/css; charset=UTF-8\r\n");
              url = path;
            } else if (leaf == "png") {
              document.append("Content-Type: image/png\r\n");
              url = path;
            } else if (leaf == "mp3") {
              document.append("Content-Type: audio/mpeg\r\n");
              url = path;
            }
            stringstream content;
            if (url.length() > 0) {
              content << ifstream (string("app/pub").append(url)).rdbuf();
            } else {
              srand(time(0));
              if (rand() % 21) {
                document = "HTTP/1.1 404 Not Found\r\n";
                content << "Today, is a beautiful day.";
              } else { // Humans! go to any random url to check your luck
                document = "HTTP/1.1 418 I'm a teapot\r\n";
                content << "Today, is your lucky day!";
              }
            }
            document.append("Content-Length: ").append(to_string(content.str().length())).append("\r\n\r\n").append(content.str());
            res->write(document.data(), document.length());
          }
        });
        uiGroup->onMessage([sess](uWS::WebSocket<uWS::SERVER> *webSocket, const char *message, size_t length, uWS::OpCode opCode) {
          if (length > 1) {
            string m = string(message, length).substr(2, length-2);
            json v;
            if (length > 2 && (m[0] == '[' || m[0] == '{')) v = json::parse(m.data());
            if (sess->cb.find(string(message, 2)) != sess->cb.end()) {
              json reply = (*sess->cb[string(message, 2)])(v);
              if (!reply.is_null() && uiBIT::SNAP == (uiBIT)message[0])
                webSocket->send(string(message, 2).append(reply.dump()).data(), uWS::OpCode::TEXT);
            }
          }
        });
        uS::TLS::Context c = uS::TLS::createContext("dist/sslcert/server.crt", "dist/sslcert/server.key", "");
        if ((access("dist/sslcert/server.crt", F_OK) != -1) && (access("dist/sslcert/server.key", F_OK) != -1) && hub.listen(port, c, 0, uiGroup))
          cout << FN::uiT() << "UI" << RWHITE << " ready over " << RYELLOW << "HTTPS" << RWHITE << " on external port " << RYELLOW << to_string(port) << RWHITE << "." << endl;
        else if (hub.listen(port, nullptr, 0, uiGroup))
          cout << FN::uiT() << "UI" << RWHITE << " ready over " << RYELLOW << "HTTP" << RWHITE << " on external port " << RYELLOW << to_string(port) << RWHITE << "." << endl;
        else { cout << FN::uiT() << "IU" << RRED << " Errrror: " << BRED << "Use another UI port number, " << RRED << to_string(port) << BRED << " seems already in use by:" << endl << BPURPLE << FN::output(string("netstat -anp 2>/dev/null | grep ").append(to_string(port))) << endl; exit(1); }
        UI::uiSnap(uiTXT::ApplicationState, &onSnapApp);
        UI::uiSnap(uiTXT::Notepad, &onSnapNote);
        UI::uiHand(uiTXT::Notepad, &onHandNote);
        UI::uiSnap(uiTXT::ToggleConfigs, &onSnapOpt);
        UI::uiHand(uiTXT::ToggleConfigs, &onHandOpt);
        CF::external();
      };
      static void uiSnap(uiTXT k, uiCb cb) {
        uiOn(uiBIT::SNAP, k, cb);
      };
      static void uiHand(uiTXT k, uiCb cb) {
        uiOn(uiBIT::MSG, k, cb);
      };
      static void uiSend(uiTXT k, json o, bool h = false) {
        uiSess *sess = (uiSess *) uiGroup->getUserData();
        if (sess->u == 0) return;
        if (k == uiTXT::MarketData) {
          if (uiT_MKT+369 > FN::T()) return;
          uiT_MKT = FN::T();
        }
        if (h) uiHold(k, o);
        else uiUp(k, o);
      };
      static void delay(double delayUI) {
        ui_delayUI = delayUI;
        uiSess *sess = (uiSess *) uiGroup->getUserData();
        sess->D.clear();
        thread([&]() {
          unsigned int uiThread_ = ++uiThread;
          double k = ui_delayUI;
          int timeout = k ? (int)(k*1e+3) : 6e+4;
          while (uiThread_ == uiThread) {
            if (k) appPush();
            else appState();
            this_thread::sleep_for(chrono::milliseconds(timeout));
          }
        }).detach();
      };
    private:
      static json onSnapApp(json z) {
        return { serverState() };
      };
      static json onSnapNote(json z) {
        return { uiNOTE };
      };
      static json onHandNote(json k) {
        if (!k.is_null() and k.size())
          uiNOTE = k.at(0);
        return {};
      };
      static json onSnapOpt(json z) {
        return { uiVisibleOpt };
      };
      static json onHandOpt(json k) {
        if (!k.is_null() and k.size())
          uiVisibleOpt = k.at(0);
        return {};
      };
      static void uiUp(uiTXT k, json o) {
        string m = string(1, (char)uiBIT::MSG).append(string(1, (char)k)).append(o.is_null() ? "" : o.dump());
        uiGroup->broadcast(m.data(), m.length(), uWS::OpCode::TEXT);
      };
      static void uiOn(uiBIT k_, uiTXT _k, uiCb cb) {
        uiSess *sess = (uiSess *) uiGroup->getUserData();
        string k = string(1, (char)k_).append(string(1, (char)_k));
        if (sess->cb.find(k) != sess->cb.end()) { cout << FN::uiT() << "Use only a single unique message handler for each \"" << k << "\" event" << endl; exit(1); }
        sess->cb[k] = cb;
      };
      static void uiHold(uiTXT k, json o) {
        bool isOSR = k == uiTXT::OrderStatusReports;
        if (isOSR && mORS::New == (mORS)o.value("orderStatus", 0)) return (void)++uiOSR_1m;
        if (!ui_delayUI) return uiUp(k, o);
        uiSess *sess = (uiSess *) uiGroup->getUserData();
        if (sess->D.find(k) != sess->D.end() && sess->D[k].size() > 0) {
          if (!isOSR) sess->D[k].clear();
          else for (vector<json>::iterator it = sess->D[k].begin(); it != sess->D[k].end();)
            if (it->value("orderId", "") == o.value("orderId", ""))
              it = sess->D[k].erase(it);
            else ++it;
        }
        sess->D[k].push_back(o);
      };
      static json serverState() {
        time_t rawtime;
        time(&rawtime);
        return {
          {"memory", FN::memory()},
          {"hour", localtime(&rawtime)->tm_hour},
          {"freq", uiOSR_1m / 2},
          {"dbsize", DB::size()},
          {"a", A()}
        };
      };
      static void appState() {
        uiSend(uiTXT::ApplicationState, serverState());
        uiOSR_1m = 0;
      };
      static void appPush() {
        uiSess *sess = (uiSess *) uiGroup->getUserData();
        for (map<uiTXT, vector<json>>::iterator it_=sess->D.begin(); it_!=sess->D.end();) {
          if (it_->first != uiTXT::OrderStatusReports) {
            for (vector<json>::iterator it = it_->second.begin(); it != it_->second.end(); ++it)
              uiUp(it_->first, *it);
            it_ = sess->D.erase(it_);
          } else ++it_;
        }
        if (sess->D.find(uiTXT::OrderStatusReports) != sess->D.end() && sess->D[uiTXT::OrderStatusReports].size() > 0) {
          int ki = 0;
          json k;
          for (vector<json>::iterator it = sess->D[uiTXT::OrderStatusReports].begin(); it != sess->D[uiTXT::OrderStatusReports].end();) {
            k.push_back(*it);
            if (mORS::Working != (mORS)it->value("orderStatus", 0))
              it = sess->D[uiTXT::OrderStatusReports].erase(it);
            else ++it;
          }
          if (!k.is_null())
            uiUp(uiTXT::OrderStatusReports, k);
          sess->D.erase(uiTXT::OrderStatusReports);
        }
        if (uiT_1m+60000 > FN::T()) return;
        uiT_1m = FN::T();
        appState();
      };
  };
}

#endif
