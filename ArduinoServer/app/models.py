from sqlalchemy import Column, Integer, Float, DateTime
from sqlalchemy.sql import func
from .database import Base

class Reading(Base):
    __tablename__ = "readings"

    id = Column(Integer, primary_key=True, index=True)
    timestamp = Column(DateTime(timezone=True), server_default=func.now())
    temp_air = Column(Float)
    temp_wall = Column(Float)
    wilg_pow = Column(Float)
    mech = Column(Integer)
    woda = Column(Integer)
    dym = Column(Integer)
    ogien = Column(Integer)
