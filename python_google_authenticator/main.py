from fastapi import FastAPI, HTTPException
from fastapi.responses import PlainTextResponse
from google.oauth2 import id_token
from google.auth.exceptions import GoogleAuthError
from google.auth.transport import requests 
from pydantic import BaseModel

CLIENT_ID = "987307069745-gsd1drcikr8retccfcafgf3tme882ub3.apps.googleusercontent.com"; #web client id

app = FastAPI()

class TokenData(BaseModel):
    token: str

@app.post("/verify")
def verify(data: TokenData):
        try:
                payload = id_token.verify_oauth2_token(data.token,requests.Request(),CLIENT_ID)
                user_id = payload.get("sub")
                return PlainTextResponse(user_id)
        except ValueError as e:
                # Token verification failed (expired, bad signature, malformed)
                raise HTTPException(status_code=401, detail=f"Token verification failed: {str(e)}")

        except GoogleAuthError as e:
                # Issuer invalid, or other Google auth problem
                raise HTTPException(status_code=401, detail=f"Google auth error: {str(e)}")

