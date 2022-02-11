#
# https://towardsdatascience.com/dockerizing-jupyter-projects-39aad547484a
# docker build -t svg .
# docker run --rm -v $PWD:/home -p 8888:8888 svg
#
FROM continuumio/miniconda3:latest

RUN conda install \
    pycairo \
    jupyterlab

VOLUME /home
EXPOSE 8888

WORKDIR /home
CMD ["jupyter-lab","--ip=0.0.0.0","--no-browser","--allow-root"]
