package xyz.wecode.blockchain.proxy;

import java.util.ArrayList;
import java.util.Random;

public final class ProxyManager {
    private ArrayList<IPProxy> proxyList = new ArrayList<>(4);

    private ProxyManager() {
        proxyList.add(new IPProxy("96.44.187.55", 8080, "xfj", "xfj"));
        proxyList.add(new IPProxy("128.199.211.84", 8080, "xfj", "xfj"));
        proxyList.add(new IPProxy("tg.sumoo.top", 808, "pdomo", "pdomo"));
        proxyList.add(new IPProxy("tgfree1.ml", 23333, "public", "free"));
    }

    private static class Holder {
        final static ProxyManager INSTANCE = new ProxyManager();
    }

    public static ProxyManager getInstance() {
        return Holder.INSTANCE;
    }

    public IPProxy getRandom() {
        Random r = new Random();
        int n = r.nextInt(5);
        if (n < proxyList.size()) {
            return proxyList.get(n);
        }
        return proxyList.get(0);
    }

}
