/*
 *  Exercise indent depths with spaces and string gap.  Ensures optimized
 *  indent handling works correctly for all code paths.
 */

/*===
{
  "foo": "bar"
}
{
    "foo": "bar"
}
{
<abcdefghi"foo": "bar"
}
{
  "foo": {
    "foo": "bar"
  }
}
{
    "foo": {
        "foo": "bar"
    }
}
{
<abcdefghi"foo": {
<abcdefghi<abcdefghi"foo": "bar"
<abcdefghi}
}
{
  "foo": {
    "foo": {
      "foo": "bar"
    }
  }
}
{
    "foo": {
        "foo": {
            "foo": "bar"
        }
    }
}
{
<abcdefghi"foo": {
<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi"foo": "bar"
<abcdefghi<abcdefghi}
<abcdefghi}
}
{
  "foo": {
    "foo": {
      "foo": {
        "foo": "bar"
      }
    }
  }
}
{
    "foo": {
        "foo": {
            "foo": {
                "foo": "bar"
            }
        }
    }
}
{
<abcdefghi"foo": {
<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": "bar"
<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi}
<abcdefghi}
}
{
  "foo": {
    "foo": {
      "foo": {
        "foo": {
          "foo": "bar"
        }
      }
    }
  }
}
{
    "foo": {
        "foo": {
            "foo": {
                "foo": {
                    "foo": "bar"
                }
            }
        }
    }
}
{
<abcdefghi"foo": {
<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": "bar"
<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi}
<abcdefghi}
}
{
  "foo": {
    "foo": {
      "foo": {
        "foo": {
          "foo": {
            "foo": "bar"
          }
        }
      }
    }
  }
}
{
    "foo": {
        "foo": {
            "foo": {
                "foo": {
                    "foo": {
                        "foo": "bar"
                    }
                }
            }
        }
    }
}
{
<abcdefghi"foo": {
<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": "bar"
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi}
<abcdefghi}
}
{
  "foo": {
    "foo": {
      "foo": {
        "foo": {
          "foo": {
            "foo": {
              "foo": "bar"
            }
          }
        }
      }
    }
  }
}
{
    "foo": {
        "foo": {
            "foo": {
                "foo": {
                    "foo": {
                        "foo": {
                            "foo": "bar"
                        }
                    }
                }
            }
        }
    }
}
{
<abcdefghi"foo": {
<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": "bar"
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi}
<abcdefghi}
}
{
  "foo": {
    "foo": {
      "foo": {
        "foo": {
          "foo": {
            "foo": {
              "foo": {
                "foo": "bar"
              }
            }
          }
        }
      }
    }
  }
}
{
    "foo": {
        "foo": {
            "foo": {
                "foo": {
                    "foo": {
                        "foo": {
                            "foo": {
                                "foo": "bar"
                            }
                        }
                    }
                }
            }
        }
    }
}
{
<abcdefghi"foo": {
<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": "bar"
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi}
<abcdefghi}
}
{
  "foo": {
    "foo": {
      "foo": {
        "foo": {
          "foo": {
            "foo": {
              "foo": {
                "foo": {
                  "foo": "bar"
                }
              }
            }
          }
        }
      }
    }
  }
}
{
    "foo": {
        "foo": {
            "foo": {
                "foo": {
                    "foo": {
                        "foo": {
                            "foo": {
                                "foo": {
                                    "foo": "bar"
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
{
<abcdefghi"foo": {
<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": "bar"
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi}
<abcdefghi}
}
{
  "foo": {
    "foo": {
      "foo": {
        "foo": {
          "foo": {
            "foo": {
              "foo": {
                "foo": {
                  "foo": {
                    "foo": "bar"
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}
{
    "foo": {
        "foo": {
            "foo": {
                "foo": {
                    "foo": {
                        "foo": {
                            "foo": {
                                "foo": {
                                    "foo": {
                                        "foo": "bar"
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
{
<abcdefghi"foo": {
<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": "bar"
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi}
<abcdefghi}
}
{
  "foo": {
    "foo": {
      "foo": {
        "foo": {
          "foo": {
            "foo": {
              "foo": {
                "foo": {
                  "foo": {
                    "foo": {
                      "foo": "bar"
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}
{
    "foo": {
        "foo": {
            "foo": {
                "foo": {
                    "foo": {
                        "foo": {
                            "foo": {
                                "foo": {
                                    "foo": {
                                        "foo": {
                                            "foo": "bar"
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
{
<abcdefghi"foo": {
<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": "bar"
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi}
<abcdefghi}
}
{
  "foo": {
    "foo": {
      "foo": {
        "foo": {
          "foo": {
            "foo": {
              "foo": {
                "foo": {
                  "foo": {
                    "foo": {
                      "foo": {
                        "foo": "bar"
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}
{
    "foo": {
        "foo": {
            "foo": {
                "foo": {
                    "foo": {
                        "foo": {
                            "foo": {
                                "foo": {
                                    "foo": {
                                        "foo": {
                                            "foo": {
                                                "foo": "bar"
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
{
<abcdefghi"foo": {
<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": "bar"
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi}
<abcdefghi}
}
{
  "foo": {
    "foo": {
      "foo": {
        "foo": {
          "foo": {
            "foo": {
              "foo": {
                "foo": {
                  "foo": {
                    "foo": {
                      "foo": {
                        "foo": {
                          "foo": "bar"
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}
{
    "foo": {
        "foo": {
            "foo": {
                "foo": {
                    "foo": {
                        "foo": {
                            "foo": {
                                "foo": {
                                    "foo": {
                                        "foo": {
                                            "foo": {
                                                "foo": {
                                                    "foo": "bar"
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
{
<abcdefghi"foo": {
<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": "bar"
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi}
<abcdefghi}
}
{
  "foo": {
    "foo": {
      "foo": {
        "foo": {
          "foo": {
            "foo": {
              "foo": {
                "foo": {
                  "foo": {
                    "foo": {
                      "foo": {
                        "foo": {
                          "foo": {
                            "foo": "bar"
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}
{
    "foo": {
        "foo": {
            "foo": {
                "foo": {
                    "foo": {
                        "foo": {
                            "foo": {
                                "foo": {
                                    "foo": {
                                        "foo": {
                                            "foo": {
                                                "foo": {
                                                    "foo": {
                                                        "foo": "bar"
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
{
<abcdefghi"foo": {
<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": "bar"
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi}
<abcdefghi}
}
{
  "foo": {
    "foo": {
      "foo": {
        "foo": {
          "foo": {
            "foo": {
              "foo": {
                "foo": {
                  "foo": {
                    "foo": {
                      "foo": {
                        "foo": {
                          "foo": {
                            "foo": {
                              "foo": "bar"
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}
{
    "foo": {
        "foo": {
            "foo": {
                "foo": {
                    "foo": {
                        "foo": {
                            "foo": {
                                "foo": {
                                    "foo": {
                                        "foo": {
                                            "foo": {
                                                "foo": {
                                                    "foo": {
                                                        "foo": {
                                                            "foo": "bar"
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
{
<abcdefghi"foo": {
<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": "bar"
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi}
<abcdefghi}
}
{
  "foo": {
    "foo": {
      "foo": {
        "foo": {
          "foo": {
            "foo": {
              "foo": {
                "foo": {
                  "foo": {
                    "foo": {
                      "foo": {
                        "foo": {
                          "foo": {
                            "foo": {
                              "foo": {
                                "foo": "bar"
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}
{
    "foo": {
        "foo": {
            "foo": {
                "foo": {
                    "foo": {
                        "foo": {
                            "foo": {
                                "foo": {
                                    "foo": {
                                        "foo": {
                                            "foo": {
                                                "foo": {
                                                    "foo": {
                                                        "foo": {
                                                            "foo": {
                                                                "foo": "bar"
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
{
<abcdefghi"foo": {
<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": "bar"
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi}
<abcdefghi}
}
{
  "foo": {
    "foo": {
      "foo": {
        "foo": {
          "foo": {
            "foo": {
              "foo": {
                "foo": {
                  "foo": {
                    "foo": {
                      "foo": {
                        "foo": {
                          "foo": {
                            "foo": {
                              "foo": {
                                "foo": {
                                  "foo": "bar"
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}
{
    "foo": {
        "foo": {
            "foo": {
                "foo": {
                    "foo": {
                        "foo": {
                            "foo": {
                                "foo": {
                                    "foo": {
                                        "foo": {
                                            "foo": {
                                                "foo": {
                                                    "foo": {
                                                        "foo": {
                                                            "foo": {
                                                                "foo": {
                                                                    "foo": "bar"
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
{
<abcdefghi"foo": {
<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": {
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi"foo": "bar"
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi<abcdefghi}
<abcdefghi<abcdefghi}
<abcdefghi}
}
===*/

function mkobj(n) {
    var obj = { foo: 'bar' };
    while (n-- > 0) {
        obj = { foo: obj };
    }
    return obj;
}

function test() {
    // 0...16 should exercise current code paths sufficiently
    for (var i = 0; i <= 16; i++) {
        print(JSON.stringify(mkobj(i), null, 2));
        print(JSON.stringify(mkobj(i), null, 4));
        print(JSON.stringify(mkobj(i), null, '<abcdefghijklmnopq>'));  // intentionally >10 chars, will be truncated to 10
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
