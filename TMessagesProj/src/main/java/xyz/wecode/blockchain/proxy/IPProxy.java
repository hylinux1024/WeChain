package xyz.wecode.blockchain.proxy;

public class IPProxy {
    public String ipAddress;
    public int port;
    public String username;
    public String password;

    public IPProxy(String ipAddress, int port, String username, String password) {
        this.ipAddress = ipAddress;
        this.port = port;
        this.username = username;
        this.password = password;
    }


}
