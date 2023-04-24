#!/bin/bash
curl -X POST \
     -H 'Content-Type: application/json' \
     -d '{"id": "5135958352", "jsonrpc": "2.0", "method": "query", "params": {"sql": "SELECT * FROM categories WHERE name = '\''Food & Beverages'\''"}}' \
     http://127.0.0.1:57740
# -d @./request.json \
