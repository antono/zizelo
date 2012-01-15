namespace Zizelo {
  class Net {

    // public GNet.URI uri;
    public delegate void RequestCallback (DataInputStream dis);

    public Net() {
      // uri = new GNet.URI(url);
    }

    public async void fetch (string url, RequestCallback cb) throws Error {
      // var uri = new GNet.URI(url);
      // debug("HOST: %s", uri.hostname);
      // debug("PATH: %s", uri.path);
      // debug("PORT: %d", uri.port);

      var resolver = Resolver.get_default ();
      var addresses = yield resolver.lookup_by_name_async ("antono.info");
      var address = addresses.nth_data (0);
      // debug (@"resolved $(uri.hostname) to $(address)");

      var socket_address = new InetSocketAddress (address, (uint16)70);
      var client = new SocketClient ();
      var conn = yield client.connect_async (socket_address);
      // debug (@"connected to $(uri.hostname)");

      var selector = "/about\r\n";
      debug (selector);

      yield conn.output_stream.write_async (selector.data, Priority.DEFAULT);
      debug ("wrote request");

      conn.socket.set_blocking (true);

      var input = new DataInputStream (conn.input_stream);

      cb(input);
    }
  }
}
