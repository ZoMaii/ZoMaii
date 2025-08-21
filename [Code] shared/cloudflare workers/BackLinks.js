/**
 * AutoBackLinks Conceal origin Server's IP- CloudFlare Workers
 * 
 * About Bot fight mod (enabled):
 * WAF can not skip it . But Super bot fight is ok.
 * IF you want keep config now , xxx.yourdomain.xxx(NS, As zone, disabled bot fight) is ok too.
 * 
 * 
 * start your test by https://httpbin.org/get
 * 
 * CloudFlare DOC at https://developers.cloudflare.com/bots/get-started/super-bot-fight-mode/
 * CloudFlare Workers DOC at https://developers.cloudflare.com/workers/
 */

export default {
  async fetch(request,env) {
    // work config
    const debug_break_inbound = false;
    const debug_break_outbound = false;

    // ask env (Workers > Setting > Variables and Secrets)
    const inbound_AllowList = env.allowlist.inbound;
    const outbound_AllowList = env.allowlist.outbound;

    // ask client
    const clientIP = request.headers.get("CF-Connecting-IP") || "";
    const originHost = new URL(request.url).hostname;
    const urlParam = new URL(request.url).searchParams.get("url");
    let target;

    if(!debug_break_inbound){
      // Step 1 , Verify your own domain(Host)
      if (
        !inbound_AllowList.includes(clientIP) &&
        !inbound_AllowList.includes(originHost)
      ) {
        return new Response("Forbidden: Inbound not allowed", { status: 403 });
      }
    }

    // Step 2 , Check whether the parameters are complete
    if (!urlParam) {
      return new Response("Missing 'url' parameter", { status: 400 });
    }

    try {
      target = new URL(urlParam);
    } catch (e) {
      return new Response("Invalid target URL", { status: 400 });
    }

    if(!debug_break_outbound){
      // Step 3 , Verify Page is trusted
      if (!outbound_AllowList.includes(target.hostname)) {
        return new Response("Forbidden: Outbound not allowed", { status: 403 });
      }
    }


    // Step4 , return data to your client
    try{
      // Constructing request headers
      const response = await fetch(target.toString(),{
        headers: {
          "User-Agent": "BackLinksBot/1.0(Auto)"
        }
      });

      return new Response(response.body,{
        status: request.status,
        headers: response.headers,
      })

    }catch(e){
      return new Response("Proxy Error: " + e.message, { status: 502 });
    }
  }
};