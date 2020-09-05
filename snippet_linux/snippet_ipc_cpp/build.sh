g++ -g -c ipc.cpp -I. -o ipc.obj

g++ -g -c m_app.cpp -I. -o m_app.obj
g++ -g m_app.obj ipc.obj -lutil -o m_app

g++ -g -c s_app.cpp -I. -o s_app.obj
g++ -g s_app.obj ipc.obj -lutil -o s_app

g++ -g -c p_app.cpp -I. -o p_app.obj
g++ -g p_app.obj ipc.obj -lutil -o p_app

