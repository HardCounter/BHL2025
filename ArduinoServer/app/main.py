from fastapi import Depends, FastAPI, HTTPException
from sqlalchemy.orm import Session
from typing import List
from . import models, schemas
from .database import SessionLocal, engine
from fastapi.middleware.cors import CORSMiddleware

models.Base.metadata.create_all(bind=engine)

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


# Dependency
def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


@app.get("/")
def read_root():
    return {"message": "Arduino Server is running"}


@app.get("/readings/", response_model=List[schemas.Reading])
def read_readings(skip: int = 0, limit: int = 100, db: Session = Depends(get_db)):
    readings = db.query(models.Reading).order_by(models.Reading.timestamp.desc()).offset(skip).limit(limit).all()
    return readings


VALID_FIELDS = ['temp_air', 'temp_wall', 'wilg_pow', 'mech', 'woda', 'dym', 'ogien']


@app.get("/readings/{field}", response_model=List[schemas.ChartDataPoint])
def read_reading_field(field: str, skip: int = 0, limit: int = 100, db: Session = Depends(get_db)):
    if field not in VALID_FIELDS:
        raise HTTPException(status_code=404, detail=f"Field '{field}' not found")

    # 1. Subquery to get the IDs of the most recent 'limit' readings with given offset
    subquery = db.query(models.Reading.id)\
        .order_by(models.Reading.timestamp.desc())\
        .offset(skip)\
        .limit(limit)\
        .subquery()

    # 2. Main query to fetch timestamp and the desired field for those IDs, sorted chronologically
    query_result = db.query(
            models.Reading.timestamp,
            getattr(models.Reading, field).label('value')
        )\
        .join(subquery, models.Reading.id == subquery.c.id)\
        .order_by(models.Reading.timestamp.asc())\
        .all()

    return query_result
