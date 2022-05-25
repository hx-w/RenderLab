# -*- coding: utf-8 -*-
'''
provides a simple REST API for the machine learning server
with the following endpoints:
    1. `/api/edgeline/predict`
    2. `/api/uv_pivot/predict`

created in 2022-05-25 by Hex
'''

import os
import uvicorn
import pickle
from fastapi import FastAPI, HTTPException

ml_server = FastAPI()

ml_models = {}

def preset():
    global ml_models
    ml_models['edge_line'] = pickle.load(open(os.path.join('static', 'dataset', 'edge_line.pkl'), 'rb'))
    ml_models['uv_pivot'] = pickle.load(open(os.path.join('static', 'dataset', 'uv_pivot.pkl'), 'rb'))


@ml_server.get('/api/edge_line/predict')
async def predict_edgeline(u: float) -> dict:
    '''
    predicts the edge line for a given u value
    '''
    try:
        global ml_models
        v_pred = ml_models['edge_line'].predict([[u]])
        return {'v': v_pred[0]}
    except Exception as ept:
        raise HTTPException(status_code=500, detail=str(ept))

@ml_server.get('/api/uv_pivot/predict')
async def predict_uv_pivot(
    pos_u: float, pos_v: float,
    angle_u: float, angle_v: float,
    length_u: float, length_v: float
) -> dict:
    '''
    predicts the uv_pivot for a given u and v value
    '''
    try:
        global ml_models
        dist_pred = ml_models['uv_pivot'].predict([
            [pos_u, pos_v, angle_u, angle_v, length_u, length_v]
        ])
        return {'dist_xyz': list(dist_pred[0])}
    except Exception as ept:
        raise HTTPException(status_code=500, detail=str(ept))


@ml_server.get('/api/quit')
async def quit_env():
    os._exit(0)


if __name__ == '__main__':
    preset()
    uvicorn.run(ml_server, host='localhost', port=8848, log_level="info")
